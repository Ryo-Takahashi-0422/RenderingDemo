#include <stdafx.h>
#include <ResourceManager.h>
#pragma comment(lib, "winmm.lib")

ResourceManager::ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, PrepareRenderingWindow* prepareRenderingWindow) : _dev(dev), _fbxInfoManager(fbxInfoManager), _prepareRenderingWindow(prepareRenderingWindow)
{
	textureLoader = new TextureLoader;
	//ファイル形式毎のテクスチャロード処理
	textureLoader->LoadTexture();

	invIdentify.r[0].m128_f32[0] *= -1;
}

ResourceManager::~ResourceManager()
{
	_fbxInfoManager = nullptr;
	_prepareRenderingWindow = nullptr;
	textureLoader = nullptr;
	m_Camera = nullptr;

	mappedVertPos = nullptr; // unmap済
	mappedIdx = nullptr; // unmap済

	matrixBuff->Unmap(0, nullptr);
	mappedMatrix = nullptr;

	materialParamBuffContainer.clear();
	mappedPhoneContainer.clear();

	textureUploadBuff.clear();
	textureReadBuff.clear();

	mappedImgContainer.clear();
	textureMetaData.clear();
	textureImg.clear();

	//for (auto& phong : mappedPhoneContainer)
	//{
	//	phong = nullptr;
	//}

	//for (auto& container : mappedImgContainer)
	//{
	//	container = nullptr; // unmap済
	//}

	//for (auto& img : textureImg)
	//{
	//	img = nullptr;
	//}
}

HRESULT ResourceManager::Init(Camera* _camera)
{
	HRESULT result = E_FAIL;
	m_Camera = _camera;

	// OBBの頂点群およびローカル回転成分・平行移動成分を取得する
	vertexListOfOBB = _fbxInfoManager->GetIndiceAndVertexInfoOfOBB();
	auto localPosAndRotOfOBB = _fbxInfoManager->GetLocalPosAndRotOfOBB();
	auto itlPosRotOfOBB = localPosAndRotOfOBB.begin();
	for (int i = 0; i < /*localMatrixOfOBB.size()*/localPosAndRotOfOBB.size(); ++i)
	{
		// 回転成分の抽出・反映 ※Y軸(UP)回転のみ対応
		auto localRotation = itlPosRotOfOBB->second.second;
		localRotation.x = XMConvertToRadians(localRotation.x);
		localRotation.y = XMConvertToRadians(localRotation.y);
		localRotation.z = XMConvertToRadians(localRotation.z);
		XMMATRIX localRotaionYMatrix = XMMatrixRotationY(localRotation.y); // blenderと符号逆

		// 平行移動成分の抽出・反映
		localRotaionYMatrix.r[3].m128_f32[0] = itlPosRotOfOBB->second.first.x;
		localRotaionYMatrix.r[3].m128_f32[1] = itlPosRotOfOBB->second.first.y;
		localRotaionYMatrix.r[3].m128_f32[2] = itlPosRotOfOBB->second.first.z;
		localMatrix4OBB[itlPosRotOfOBB->first] = localRotaionYMatrix;
		++itlPosRotOfOBB;
	}

	// 各メッシュの回転・平行移動行列を作る
	auto localPosAndRotOfMesh = _fbxInfoManager->GetLocalPosAndRotOfMesh();
	localMatrix.resize(localPosAndRotOfMesh.size());
	auto itlPosRot = localPosAndRotOfMesh.begin();
	for (int i = 0; i < localMatrix.size(); ++i)
	{
		// 回転成分の抽出・反映 ※Y軸(UP)回転のみ対応
		auto localRotation = itlPosRot->second.second;
		localRotation.x = XMConvertToRadians(localRotation.x);
		localRotation.y = XMConvertToRadians(localRotation.y);
		localRotation.z = XMConvertToRadians(localRotation.z);
		//XMMATRIX localRotaionXMatrix = XMMatrixRotationX(localRotation.x);
		XMMATRIX localRotaionYMatrix = XMMatrixRotationY(localRotation.y); // blenderと符号逆
		//XMMATRIX localRotaionZMatrix = XMMatrixRotationZ(localRotation.z);
		localMatrix[i] = localRotaionYMatrix/*XMMatrixIdentity()*/;

		// 平行移動成分の抽出・反映
		localMatrix[i].r[3].m128_f32[0] = itlPosRot->second.first.x;
		localMatrix[i].r[3].m128_f32[1] = itlPosRot->second.first.y;
		localMatrix[i].r[3].m128_f32[2] = itlPosRot->second.first.z;

		++itlPosRot;
	}

	// メッシュの頂点情報を取得する		
	vertMap = _fbxInfoManager->GetIndiceAndVertexInfo();

	auto itFirst = vertMap.begin();
	// 各メッシュの頂点座標にローカル回転平行移動行列を乗算してワールド空間へ配置するための座標に変換
	for (int i = 0; i < vertMap.size(); ++i)
	{
		for (int j = 0; j < itFirst->second.vertices.size(); ++j)
		{
			XMFLOAT3 vertCopy = itFirst->second.vertices[j].pos;
			XMVECTOR vertexPos = XMLoadFloat3(&vertCopy);
			vertexPos = XMVector3Transform(vertexPos, localMatrix[i]);
			vertCopy.x = vertexPos.m128_f32[0];
			vertCopy.y = vertexPos.m128_f32[1];
			vertCopy.z = vertexPos.m128_f32[2];
			itFirst->second.vertices[j].pos = vertCopy;
		}
		++itFirst;
	}

	itFirst = vertMap.begin();
	// create pos container
	for (int i = 0; i < vertMap.size(); ++i)
	{
		meshVertexInfos.push_back(vertMap[i]); // OBB処理用に用意する
		for (int j = 0; j < itFirst->second.vertices.size(); ++j)
		{
			verticesPosContainer.push_back(itFirst->second.vertices[j]);
		}
		++itFirst;
	}
	// デバッグ用。コライダーのみ読み込んだ際の回避処理。
	if (verticesPosContainer.size() == 0)
	{
		FBXVertex dummy = { {0,0,0},{0,0,0},{0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} };
		verticesPosContainer.push_back(dummy);
	}

	auto vertexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertexBuffSize = verticesPosContainer.size() * sizeof(FBXVertex);
	auto vertresDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBuffSize);

	result = _dev->CreateCommittedResource
	(
		&vertexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	vertexTotalNum = verticesPosContainer.size();
	result = vertBuff->Map(0, nullptr, (void**)&mappedVertPos); // mapping
	std::copy(std::begin(verticesPosContainer), std::end(verticesPosContainer), mappedVertPos);
	vertBuff->Unmap(0, nullptr);

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファの仮想アドレス
	vbView.SizeInBytes = vertexBuffSize;//全バイト数
	vbView.StrideInBytes = sizeof(FBXVertex);//1頂点あたりのバイト数

	// index Resource
	itFirst = vertMap.begin();
	// create pos container
	for (int i = 0; i < vertMap.size(); ++i)
	{
		for (int j = 0; j < itFirst->second.indices.size(); ++j)
		{
			indexContainer.push_back(itFirst->second.indices[j]);
		}
		++itFirst;
	}

	indexNum = indexContainer.size();
	auto indiceBuffSize = indexNum * sizeof(unsigned int);
	auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);
	result = _dev->CreateCommittedResource
	(
		&vertexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&indicesDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	result = idxBuff->Map(0, nullptr, (void**)&mappedIdx); // mapping
	std::copy(std::begin(indexContainer), std::end(indexContainer), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.SizeInBytes = indiceBuffSize;
	ibView.Format = DXGI_FORMAT_R32_UINT;

	// depth DHeap, Resource
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 2; // 通常の深度マップ*2
	result = _dev->CreateDescriptorHeap
	(
		&dsvHeapDesc,
		IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	auto depthHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = _prepareRenderingWindow->GetWindowWidth();
	depthResDesc.Height = _prepareRenderingWindow->GetWindowHeight();
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS; // 深度値書き込み用
	depthResDesc.SampleDesc.Count = 1; // 1pixce/1つのサンプル
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	// sponza深度マップ用
	result = _dev->CreateCommittedResource
	(
		&depthHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	// connan深度マップ用
	result = _dev->CreateCommittedResource
	(
		&depthHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthBuff2.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	auto dsvHandle = dsvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
	// sponza深度マップ用
	_dev->CreateDepthStencilView
	(
		depthBuff.Get(),
		&dsvDesc,
		dsvHandle
	);

	// connan深度マップ用
	dsvHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	_dev->CreateDepthStencilView
	(
		depthBuff2.Get(),
		&dsvDesc,
		dsvHandle
	);

	// create rendering buffer and create RTV
	CreateRTV();

	// create upload/read texture buffer and mapping Texture and Upload them to GPU
	materialAndTexturePath = _fbxInfoManager->GetMaterialAndTexturePath();
	auto textureNum = materialAndTexturePath.size();
	auto iter = materialAndTexturePath.begin();
	int texNum = materialAndTexturePath.size();
	textureUploadBuff.resize(texNum * 5);
	textureReadBuff.resize(texNum);
	textureMetaData.resize(texNum * 5);
	textureImg.resize(texNum * 5);
	textureImgPixelValue.resize(texNum);
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// 別端末でも読み込み出来るようにテクスチャファイルパスを書き換える。テクスチャはC:\Users\<usernamee>\Documents\RenderingDemoRebuild\textureに格納している
	std::string texturePath = Utility::GetTextureFilepath();
	for (auto& path : materialAndTexturePath)
	{
		auto index = path.second.rfind("\\");

		path.second = path.second.erase(0, index);
		path.second = texturePath + path.second;
	}

	mappedImgContainer.resize(texNum * 5);
	for (int i = 0; i < materialAndTexturePath.size(); ++i)
	{
		CreateUploadAndReadBuff4Texture(iter->second, i);
		++iter;
	}

	// create matrix buffer and mapping matrix, create CBV
	CreateAndMapResources(textureNum); // mapping

	// get animationInfo
	animationNameAndBoneNameWithTranslationMatrix = _fbxInfoManager->GetAnimationNameAndBoneNameWithTranslationMatrix();
	if (animationNameAndBoneNameWithTranslationMatrix.size() != 0)
	{
		isAnimationModel = true;
	}
}

HRESULT ResourceManager::CreateRTV()
{
	// create buffer
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	auto depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = _prepareRenderingWindow->GetWindowWidth();
	resDesc.Height = _prepareRenderingWindow->GetWindowHeight();
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	auto result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(renderingBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(renderingBuff2.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(normalMapBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(normalMapBuff2.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	// create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV用ディスクリプタヒープ
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 4; // ★★★
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK) return result;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateRenderTargetView//リソースデータ(_backBuffers)にアクセスするためのレンダーターゲットビューをhandleアドレスに作成
	(
		renderingBuff.Get(),//レンダーターゲットを表す ID3D12Resource オブジェクトへのポインター
		&rtvDesc,//レンダー ターゲット ビューを記述する D3D12_RENDER_TARGET_VIEW_DESC 構造体へのポインター。
		handle//新しく作成されたレンダーターゲットビューが存在する宛先を表す CPU 記述子ハンドル(ヒープ上のアドレス)
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	_dev->CreateRenderTargetView//リソースデータ(_backBuffers)にアクセスするためのレンダーターゲットビューをhandleアドレスに作成
	(
		renderingBuff2.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 法線画像 スレッド1
	_dev->CreateRenderTargetView
	(
		normalMapBuff.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 法線画像 スレッド2
	_dev->CreateRenderTargetView
	(
		normalMapBuff2.Get(),
		&rtvDesc,
		handle
	);

	return S_OK;
}

HRESULT ResourceManager::CreateAndMapResources(size_t textureNum)
{
	//Mapping WVP Matrix
	auto wvpHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto wvpResdesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(FBXSceneMatrix) + 0xff) & ~0xff);

	auto result = _dev->CreateCommittedResource
	(
		&wvpHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&wvpResdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(matrixBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	matrixBuff->Map(0, nullptr, (void**)&mappedMatrix); // mapping

	mappedMatrix->world = m_Camera->GetWorld();
	mappedMatrix->view = m_Camera->GetView();
	mappedMatrix->proj = m_Camera->GetProj();

	mappedMatrix->rotation = XMMatrixIdentity();

	auto bonesInitialPostureMatrixMap = _fbxInfoManager->GetBonesInitialPostureMatrix();
	XMVECTOR det;
	invBonesInitialPostureMatrixMap.resize(bonesInitialPostureMatrixMap.size());
	for (int i = 0; i < bonesInitialPostureMatrixMap.size(); ++i)
	{
		invBonesInitialPostureMatrixMap[i] = XMMatrixInverse(&det, bonesInitialPostureMatrixMap[i]);
	}

	// Mapping Phong Material Parameters
	phongInfos = _fbxInfoManager->GetPhongMaterialParamertInfo();
	auto phongHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto phongResdesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(PhongInfo)/* * phongInfos.size()*/ + 0xff) & ~0xff);

	materialParamBuffContainer.resize(phongInfos.size());
	mappedPhoneContainer.resize(phongInfos.size());
	for (int i = 0; i < phongInfos.size(); ++i)
	{
		//materialParamBuffContainer[i] = nullptr;
		auto& resource = materialParamBuffContainer[i];
		auto phongHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto phongResdesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(PhongInfo)/* * phongInfos.size()*/ + 0xff) & ~0xff);

		result = _dev->CreateCommittedResource
		(
			&phongHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&phongResdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
			nullptr,
			IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
		);
		if (result != S_OK) return result;
		resource->Map(0, nullptr, (void**)&mappedPhoneContainer[i]);
	}

	// create view
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = matrixBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = matrixBuff->GetDesc().Width;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; // SRV用ディスクリプタヒープ
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 13 + phongInfos.size() + textureNum; // 1:Matrix(world, view, proj)(1), 2-3:rendering result(1),(2), 4-5:depth*2, 6-11:skyとImGuiとsunとairとvsmとshadowmap, 12-13 法線画像 thread1,2 14-x:phongInfosサイズ(読み込むモデルにより変動), x-:texture数(読み込むモデルにより変動)
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;

	descriptorNum = srvHeapDesc.NumDescriptors; // 後付けのSky設定で利用

	result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf()));
	if (result != S_OK) return result;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//  1:Matrix(world, view, proj)

	_dev->CreateConstantBufferView
	(
		&cbvDesc,
		handle
	);

	// model rendering result
	handle.ptr += inc;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// 2:Thread1 描画先
	_dev->CreateShaderResourceView
	(
		renderingBuff.Get(),
		&srvDesc,
		handle
	);

	// 3:Thread2 描画先
	handle.ptr += inc;
	_dev->CreateShaderResourceView
	(
		renderingBuff2.Get(),
		&srvDesc,
		handle
	);

	handle.ptr += inc;
	// 4:sponzaデプスマップ用
	D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		depthBuff.Get(),
		&depthSRVDesc,
		handle
	);

	// 5:connanデプスマップ用
	handle.ptr += inc;
	_dev->CreateShaderResourceView
	(
		depthBuff2.Get(),
		&depthSRVDesc,
		handle
	);

	handle.ptr += inc * 7; // sky, ImGui,sun, Air, vsm, shadowmap分空けておく

	// 12:法線画像 スレッド1描画先
	_dev->CreateShaderResourceView
	(
		normalMapBuff.Get(),
		&srvDesc,
		handle
	);

	// 13:法線画像 スレッド2描画先
	handle.ptr += inc;
	_dev->CreateShaderResourceView
	(
		normalMapBuff2.Get(),
		&srvDesc,
		handle
	);

	// 14-x:Phong Material Parameters
	for (int i = 0; i < phongInfos.size(); ++i)
	{
		auto& resource = materialParamBuffContainer[i];
		handle.ptr += inc;

		D3D12_CONSTANT_BUFFER_VIEW_DESC phongCBVDesc = {};
		phongCBVDesc.BufferLocation = resource->GetGPUVirtualAddress();
		phongCBVDesc.SizeInBytes = resource->GetDesc().Width;
		_dev->CreateConstantBufferView
		(
			&phongCBVDesc,
			handle
		);
	}

	// x-:Texture Read Buffers(n=textureNum)
	D3D12_SHADER_RESOURCE_VIEW_DESC textureSRVDesc = {};
	textureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D/*D3D12_SRV_DIMENSION_TEXTURE2DARRAY*/;
	textureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	textureSRVDesc.Texture2D.MipLevels = 5; // pix上で確認したが、当然viewとしての登録情報なのでMipレベルが設定される

	//textureSRVDesc.Texture2DArray.MipLevels = 4;
	for (int i = 0; i < textureNum; ++i)
	{
		auto& resource = textureReadBuff[i];
		handle.ptr += inc;

		textureSRVDesc.Format = textureReadBuff[i]->GetDesc().Format;

		// Temporary : Accessoriesのcolor以外のjpegがSRGBになっている。Photoshop導入して変更出来るようにするまでこのまま。
		//if (i == 1 || i == 2 || i == 3)
		//{
		//	textureSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//}


		_dev->CreateShaderResourceView
		(
			resource.Get(),
			&textureSRVDesc,
			handle
		);
	}

	return S_OK;
}

void ResourceManager::CreateUploadAndReadBuff4Texture(std::string texturePath, int iterationNum)
{
	// create resource
	struct _stat s = {};

	std::string filePath = "";
	filePath = texturePath;

	auto wTexPath = Utility::GetWideStringFromSring(filePath);
	auto extention = Utility::GetExtension(filePath);

	if (!textureLoader->GetTable().count(extention))
	{
		std::cout << "読み込めないテクスチャが存在します" << std::endl;
		return;
	}

	auto imgIndex = iterationNum * 5;
	textureMetaData[imgIndex] = new TexMetadata;
	auto result = textureLoader->GetTable()[extention](wTexPath, textureMetaData[imgIndex], scratchImg/*scratchImageContainer[iterationNum]*/);

	if (scratchImg.GetImage(0, 0, 0) == nullptr) return;
	ScratchImage mipChain;
	GenerateMipMaps(scratchImg.GetImages(), scratchImg.GetImageCount(),
		scratchImg.GetMetadata(), TEX_FILTER_DEFAULT, 5, mipChain);

	//result = textureLoader->GetTable()[extention](wTexPath, textureMetaData[iterationNum], mipChain/*scratchImageContainer[iterationNum]*/);


	textureMetaData[imgIndex]->mipLevels = 1;
	textureImg[imgIndex] = nullptr;
	textureImg[imgIndex] = new Image;
	textureImg[imgIndex]->pixels = scratchImg.GetImage(0, 0, 0)->pixels;
	textureImg[imgIndex]->rowPitch = scratchImg.GetImage(0, 0, 0)->rowPitch;
	textureImg[imgIndex]->format = scratchImg.GetImage(0, 0, 0)->format;
	textureImg[imgIndex]->width = scratchImg.GetImage(0, 0, 0)->width;
	textureImg[imgIndex]->height = scratchImg.GetImage(0, 0, 0)->height;
	textureImg[imgIndex]->slicePitch = scratchImg.GetImage(0, 0, 0)->slicePitch;

	//auto k = mipChain.GetImage(3,0,0); // miplevelが4、元サイズ1kなら、0:1k、1:0.5k、2:0.25k、3:0.125kがそれぞれ生成されmipchain変数内に格納されている。
	auto k = mipChain.GetImages();

	// if exist texture
	if (_stat(filePath.c_str(), &s) == 0)
	{
		std::tie(textureUploadBuff[imgIndex], textureReadBuff[iterationNum]) = CreateD3DX12ResourceBuffer::LoadTextureFromFile(_dev, textureMetaData[imgIndex], textureImg[imgIndex]);

		textureImg[imgIndex + 1] = nullptr;
		textureImg[imgIndex + 1] = new Image;
		textureImg[imgIndex + 1]->pixels = mipChain.GetImage(1, 0, 0)->pixels;
		textureImg[imgIndex + 1]->rowPitch = mipChain.GetImage(1, 0, 0)->rowPitch;
		textureImg[imgIndex + 1]->format = mipChain.GetImage(1, 0, 0)->format;
		textureImg[imgIndex + 1]->width = mipChain.GetImage(1, 0, 0)->width;
		textureImg[imgIndex + 1]->height = mipChain.GetImage(1, 0, 0)->height;
		textureImg[imgIndex + 1]->slicePitch = mipChain.GetImage(1, 0, 0)->slicePitch;

		textureMetaData[imgIndex + 1] = new TexMetadata;
		textureMetaData[imgIndex + 1]->width = textureImg[imgIndex + 1]->width;
		textureMetaData[imgIndex + 1]->height = textureImg[imgIndex + 1]->height;
		textureMetaData[imgIndex + 1]->depth = 1;
		textureMetaData[imgIndex + 1]->arraySize = 1;
		textureMetaData[imgIndex + 1]->format = textureImg[imgIndex + 1]->format;
		textureMetaData[imgIndex + 1]->mipLevels = 1;
		textureMetaData[imgIndex + 1]->dimension = TEX_DIMENSION_TEXTURE2D;


		textureUploadBuff[imgIndex + 1] = CreateD3DX12ResourceBuffer::LoadTextureFromFile4UploadFile(_dev, textureImg[imgIndex + 1]);

		textureImg[imgIndex + 2] = nullptr;
		textureImg[imgIndex + 2] = new Image;
		textureImg[imgIndex + 2]->pixels = mipChain.GetImage(2, 0, 0)->pixels;
		textureImg[imgIndex + 2]->rowPitch = mipChain.GetImage(2, 0, 0)->rowPitch;
		textureImg[imgIndex + 2]->format = mipChain.GetImage(2, 0, 0)->format;
		textureImg[imgIndex + 2]->width = mipChain.GetImage(2, 0, 0)->width;
		textureImg[imgIndex + 2]->height = mipChain.GetImage(2, 0, 0)->height;
		textureImg[imgIndex + 2]->slicePitch = mipChain.GetImage(2, 0, 0)->slicePitch;

		textureMetaData[imgIndex + 2] = new TexMetadata;
		textureMetaData[imgIndex + 2]->width = textureImg[imgIndex + 2]->width;
		textureMetaData[imgIndex + 2]->height = textureImg[imgIndex + 2]->height;
		textureMetaData[imgIndex + 2]->depth = 1;
		textureMetaData[imgIndex + 2]->arraySize = 1;
		textureMetaData[imgIndex + 2]->format = textureImg[imgIndex + 2]->format;
		textureMetaData[imgIndex + 2]->mipLevels = 1;
		textureMetaData[imgIndex + 2]->dimension = TEX_DIMENSION_TEXTURE2D;

		textureUploadBuff[imgIndex + 2] = CreateD3DX12ResourceBuffer::LoadTextureFromFile4UploadFile(_dev, textureImg[imgIndex + 2]);

		textureImg[imgIndex + 3] = nullptr;
		textureImg[imgIndex + 3] = new Image;
		textureImg[imgIndex + 3]->pixels = mipChain.GetImage(3, 0, 0)->pixels;
		textureImg[imgIndex + 3]->rowPitch = mipChain.GetImage(3, 0, 0)->rowPitch;
		textureImg[imgIndex + 3]->format = mipChain.GetImage(3, 0, 0)->format;
		textureImg[imgIndex + 3]->width = mipChain.GetImage(3, 0, 0)->width;
		textureImg[imgIndex + 3]->height = mipChain.GetImage(3, 0, 0)->height;
		textureImg[imgIndex + 3]->slicePitch = mipChain.GetImage(3, 0, 0)->slicePitch;

		textureMetaData[imgIndex + 3] = new TexMetadata;
		textureMetaData[imgIndex + 3]->width = textureImg[imgIndex + 3]->width;
		textureMetaData[imgIndex + 3]->height = textureImg[imgIndex + 3]->height;
		textureMetaData[imgIndex + 3]->depth = 1;
		textureMetaData[imgIndex + 3]->arraySize = 1;
		textureMetaData[imgIndex + 3]->format = textureImg[imgIndex + 3]->format;
		textureMetaData[imgIndex + 3]->mipLevels = 1;
		textureMetaData[imgIndex + 3]->dimension = TEX_DIMENSION_TEXTURE2D;

		textureUploadBuff[imgIndex + 3] = CreateD3DX12ResourceBuffer::LoadTextureFromFile4UploadFile(_dev, textureImg[imgIndex + 3]);


		textureImg[imgIndex + 4] = nullptr;
		textureImg[imgIndex + 4] = new Image;
		textureImg[imgIndex + 4]->pixels = mipChain.GetImage(4, 0, 0)->pixels;
		textureImg[imgIndex + 4]->rowPitch = mipChain.GetImage(4, 0, 0)->rowPitch;
		textureImg[imgIndex + 4]->format = mipChain.GetImage(4, 0, 0)->format;
		textureImg[imgIndex + 4]->width = mipChain.GetImage(4, 0, 0)->width;
		textureImg[imgIndex + 4]->height = mipChain.GetImage(4, 0, 0)->height;
		textureImg[imgIndex + 4]->slicePitch = mipChain.GetImage(4, 0, 0)->slicePitch;

		textureMetaData[imgIndex + 4] = new TexMetadata;
		textureMetaData[imgIndex + 4]->width = textureImg[imgIndex + 4]->width;
		textureMetaData[imgIndex + 4]->height = textureImg[imgIndex + 4]->height;
		textureMetaData[imgIndex + 4]->depth = 1;
		textureMetaData[imgIndex + 4]->arraySize = 1;
		textureMetaData[imgIndex + 4]->format = textureImg[imgIndex + 4]->format;
		textureMetaData[imgIndex + 4]->mipLevels = 1;
		textureMetaData[imgIndex + 4]->dimension = TEX_DIMENSION_TEXTURE2D;

		textureUploadBuff[imgIndex + 4] = CreateD3DX12ResourceBuffer::LoadTextureFromFile4UploadFile(_dev, textureImg[imgIndex + 4]);
	}

	else
	{
		std::tie(textureUploadBuff[imgIndex], textureReadBuff[iterationNum]) = std::forward_as_tuple(nullptr, nullptr);

		textureUploadBuff[imgIndex + 1] = nullptr;
		textureUploadBuff[imgIndex + 2] = nullptr;
		textureUploadBuff[imgIndex + 3] = nullptr;
		textureUploadBuff[imgIndex + 4] = nullptr;
	}

	// mapping
	if (textureUploadBuff[imgIndex] == nullptr) return;

	for (int index = 0; index < 5; ++index)
	{
		auto srcAddress = textureImg[imgIndex + index]->pixels;
		auto rowPitch = Utility::AlignmentSize(textureImg[imgIndex + index]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		result = textureUploadBuff[imgIndex + index]->Map(0, nullptr, (void**)&mappedImgContainer[imgIndex + index]);

		// img:元データの初期アドレス(srcAddress)を元ピッチ分オフセットしながら、補正したピッチ個分(rowPitch)のアドレスを
		// mappedImgContainer[i]にその数分(rowPitch)オフセットを繰り返しつつコピーしていく
		for (int i = 0; i < textureImg[imgIndex + index]->height; ++i)
		{
			std::copy_n(srcAddress, rowPitch, mappedImgContainer[imgIndex + index]);
			srcAddress += textureImg[imgIndex + index]->rowPitch;
			mappedImgContainer[imgIndex + index] += rowPitch;
		}

		textureUploadBuff[imgIndex + index]->Unmap(0, nullptr);
	}
}

void ResourceManager::ClearReference()
{
	_fbxInfoManager = nullptr;
	delete _fbxInfoManager;
	delete textureLoader;
	_prepareRenderingWindow = nullptr;
	delete _prepareRenderingWindow;
}

void ResourceManager::PlayAnimation()
{
	_startTime = timeGetTime();
}

void ResourceManager::MotionUpdate(std::string motionName, unsigned int maxFrameNum)
{
	elapsedTime = timeGetTime() - _startTime; // 経過時間を測定して格納
	frameNo = 30 * (elapsedTime / 1000.0f);

	// 現在フレーム数 >= 最大フレーム数にすることで、同一モーションが最終フレームで開始フレームと滑らかに繋がっている場合はシームレスになる
	if (frameNo >= maxFrameNum)
	{
		PlayAnimation();
		frameNo = 0;
	}

	//animationNameAndBoneNameWithTranslationMatrix = _fbxInfoManager->GetAnimationNameAndBoneNameWithTranslationMatrix();
	//XMVECTOR det;

	// 初期姿勢の逆行列と、フレーム毎姿勢行列にX軸反転行列を掛けたものを乗算して、アニメーションさせる
	for (int i = 0; i < animationNameAndBoneNameWithTranslationMatrix[motionName].size(); ++i)
	{
		mappedMatrix->bones[i] = invBonesInitialPostureMatrixMap[i] * (animationNameAndBoneNameWithTranslationMatrix[motionName][i][frameNo] * invIdentify);
	}
}

void ResourceManager::SetSkyResourceAndCreateView(ComPtr<ID3D12Resource> _skyResource)
{
	skyBuffer = _skyResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	handle.ptr += inc * 5;

	_dev->CreateShaderResourceView
	(
		skyBuffer.Get(),
		&srvDesc,
		handle
	);
}

void ResourceManager::SetImGuiResourceAndCreateView(ComPtr<ID3D12Resource> _imguiResource)
{
	imguiBuffer = _imguiResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	handle.ptr += inc * 6;

	_dev->CreateShaderResourceView
	(
		imguiBuffer.Get(),
		&srvDesc,
		handle
	);
}

void ResourceManager::SetSunResourceAndCreateView(ComPtr<ID3D12Resource> _sunResource)
{
	sunBuffer = _sunResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	handle.ptr += inc * 7;

	_dev->CreateShaderResourceView
	(
		sunBuffer.Get(),
		&srvDesc,
		handle
	);
}

void ResourceManager::SetAirResourceAndCreateView(ComPtr<ID3D12Resource> _airResource)
{
	airBuffer = _airResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	handle.ptr += inc * 8;

	_dev->CreateShaderResourceView
	(
		airBuffer.Get(),
		&srvDesc,
		handle
	);
}

void ResourceManager::SetVSMResourceAndCreateView(ComPtr<ID3D12Resource> _vsmResource)
{
	vsmBuffer = _vsmResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle.ptr += inc * 9;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = /*DXGI_FORMAT_R32_FLOAT*/DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	_dev->CreateShaderResourceView
	(
		vsmBuffer.Get(),
		&srvDesc,
		handle
	);

}

void ResourceManager::SetShadowResourceAndCreateView(ComPtr<ID3D12Resource> _shadowResource)
{
	shadowmapBuffer = _shadowResource;

	auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	handle.ptr += inc * 10;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	_dev->CreateShaderResourceView
	(
		shadowmapBuffer.Get(),
		&srvDesc,
		handle
	);

}

void ResourceManager::SetSceneInfo(XMMATRIX _shadowPosMatrix, XMMATRIX _shadowPosInvMatrix, XMMATRIX _shadowView, XMFLOAT3 _eyePos, XMFLOAT3 _sunDIr)
{
	mappedMatrix->shadowPosMatrix = _shadowPosMatrix;
	mappedMatrix->shadowPosInvMatrix = _shadowPosInvMatrix;
	mappedMatrix->shadowView = _shadowView;
	mappedMatrix->eyePos = _eyePos;
	mappedMatrix->sunDIr = _sunDIr;
}

void ResourceManager::SetProjMatrix(XMMATRIX _proj)
{
	mappedMatrix->oProj = _proj;
}

void ResourceManager::SetAirDraw(bool _isDraw)
{
	mappedMatrix->airDraw = _isDraw;
}

void ResourceManager::SetBRDFSpecularDraw(bool _isDraw)
{
	mappedMatrix->brdfSpecularDraw = _isDraw;
}

void ResourceManager::SetLightPos1(XMFLOAT3 pos)
{
	mappedMatrix->plPos1 = pos;
}

void ResourceManager::SetLightPos2(XMFLOAT3 pos)
{
	mappedMatrix->plPos2 = pos;
}
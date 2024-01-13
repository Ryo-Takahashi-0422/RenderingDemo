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


HRESULT ResourceManager::Init()
{
	HRESULT result = E_FAIL;

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
	dsvHeapDesc.NumDescriptors = 1; // 深度 + lightmap + AO用
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

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	// 深度マップ用
	_dev->CreateDepthStencilView
	(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap.Get()->GetCPUDescriptorHandleForHeapStart()
	);

	// create rendering buffer and create RTV
	CreateRTV();

	// create upload/read texture buffer and mapping Texture and Upload them to GPU
	materialAndTexturePath = _fbxInfoManager->GetMaterialAndTexturePath();
	auto textureNum = materialAndTexturePath.size();
	auto iter = materialAndTexturePath.begin();
	int texNum = materialAndTexturePath.size();
	textureUploadBuff.resize(texNum);
	textureReadBuff.resize(texNum);
	textureMetaData.resize(texNum);
	textureImg.resize(texNum);
	textureImgPixelValue.resize(texNum);
	CoInitializeEx(0, COINIT_MULTITHREADED);

	mappedImgContainer.resize(texNum);
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

	// create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV用ディスクリプタヒープ
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 2; // ★★★
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
		renderingBuff2.Get(),//レンダーターゲットを表す ID3D12Resource オブジェクトへのポインター
		&rtvDesc,//レンダー ターゲット ビューを記述する D3D12_RENDER_TARGET_VIEW_DESC 構造体へのポインター。
		handle//新しく作成されたレンダーターゲットビューが存在する宛先を表す CPU 記述子ハンドル(ヒープ上のアドレス)
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
		
	auto worldMat = XMMatrixIdentity();
	auto angle = XMMatrixRotationY(M_PI);
	//worldMat *= angle; // モデルが後ろ向きなので180°回転して調整

	//ビュー行列の生成・乗算
	XMFLOAT3 eye(0, 1.5, 2.3);
	XMFLOAT3 target(0, 1.5, 0);
	//XMFLOAT3 eye(0, 100, /*0.01*/10);
	//XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH
	(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);
	
	//プロジェクション(射影)行列の生成・乗算
	auto projMat = XMMatrixPerspectiveFovLH
	(
		XM_PIDIV2, // 画角90°
		static_cast<float>(_prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(_prepareRenderingWindow->GetWindowWidth()),
		1.0, // ニア―クリップ
		3000.0 // ファークリップ
	);

	matrixBuff->Map(0, nullptr, (void**)&mappedMatrix); // mapping
	
	mappedMatrix->world = worldMat;
	mappedMatrix->view = viewMat;
	mappedMatrix->proj = projMat;
    
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
	srvHeapDesc.NumDescriptors = 3 + phongInfos.size() + textureNum; // 1:Matrix(world, view, proj)(1), 2-3:rendering result(1),(2), 4:phongInfosサイズ(読み込むモデルにより変動), 5:texture数(読み込むモデルにより変動)
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;

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

	// 2:model rendering result
	handle.ptr += inc;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	_dev->CreateShaderResourceView
	(
		renderingBuff.Get(),
		&srvDesc,
		handle
	);

	handle.ptr += inc; // ★★★これを追加したらテクスチャバグ発生
	_dev->CreateShaderResourceView
	(
		renderingBuff2.Get(),
		&srvDesc,
		handle
	);

	// 3:Phong Material Parameters
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

	// 4:Texture Read Buffers(n=textureNum)
	D3D12_SHADER_RESOURCE_VIEW_DESC textureSRVDesc = {};
	textureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	textureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	textureSRVDesc.Texture2D.MipLevels = 1;
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

	textureMetaData[iterationNum] = new TexMetadata;
	auto result = textureLoader->GetTable()[extention](wTexPath, textureMetaData[iterationNum], scratchImg/*scratchImageContainer[iterationNum]*/);

	if (scratchImg.GetImage(0, 0, 0) == nullptr) return;
	
	textureImg[iterationNum] = nullptr;
	textureImg[iterationNum] = new Image;
	textureImg[iterationNum]->pixels = scratchImg.GetImage(0, 0, 0)->pixels;
	textureImg[iterationNum]->rowPitch = scratchImg.GetImage(0, 0, 0)->rowPitch;
	textureImg[iterationNum]->format = scratchImg.GetImage(0, 0, 0)->format;
	textureImg[iterationNum]->width = scratchImg.GetImage(0, 0, 0)->width;
	textureImg[iterationNum]->height = scratchImg.GetImage(0, 0, 0)->height;
	textureImg[iterationNum]->slicePitch = scratchImg.GetImage(0, 0, 0)->slicePitch;

	// if exist texture
	if (_stat(filePath.c_str(), &s) == 0)
	{
		std::tie(textureUploadBuff[iterationNum], textureReadBuff[iterationNum]) = CreateD3DX12ResourceBuffer::LoadTextureFromFile(_dev, textureMetaData[iterationNum], textureImg[iterationNum], filePath);
	}

	else
	{
		std::tie(textureUploadBuff[iterationNum], textureReadBuff[iterationNum]) = std::forward_as_tuple(nullptr, nullptr);
	}	

	// mapping
	if (textureUploadBuff[iterationNum] == nullptr) return;

	auto srcAddress = textureImg[iterationNum]->pixels;
	auto rowPitch = Utility::AlignmentSize(textureImg[iterationNum]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	result = textureUploadBuff[iterationNum]->Map(0, nullptr, (void**)&mappedImgContainer[iterationNum]);

	// img:元データの初期アドレス(srcAddress)を元ピッチ分オフセットしながら、補正したピッチ個分(rowPitch)のアドレスを
	// mappedImgContainer[i]にその数分(rowPitch)オフセットを繰り返しつつコピーしていく
	for (int i = 0; i < textureImg[iterationNum]->height; ++i)
	{
		std::copy_n(srcAddress, rowPitch, mappedImgContainer[iterationNum]);
		srcAddress += textureImg[iterationNum]->rowPitch;
		mappedImgContainer[iterationNum] += rowPitch;
	}

	textureUploadBuff[iterationNum]->Unmap(0, nullptr);
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
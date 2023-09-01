#include <stdafx.h>
#include <ResourceManager.h>

ResourceManager::ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, PrepareRenderingWindow* prepareRenderingWindow) : _dev(dev), _fbxInfoManager(fbxInfoManager), _prepareRenderingWindow(prepareRenderingWindow){}

HRESULT ResourceManager::Init()
{
	HRESULT result = E_FAIL;

	// vertex Resource		
	auto vertMap = _fbxInfoManager->GetIndiceAndVertexInfo();
	auto itFirst = vertMap.begin();
	// create pos container
	for (int i = 0; i < vertMap.size(); ++i)
	{		
		for (int j = 0; j < itFirst->second.vertices.size(); ++j)
		{
			verticesPosContainer.push_back(itFirst->second.vertices[j]);
		}
		++itFirst;
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

	// create matrix buffer and mapping matrix, create CBV
	CreateAndMapMatrix(); // mapping

	// ClearReference();
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

	// create RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV用ディスクリプタヒープ
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK) return result;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	_dev->CreateRenderTargetView//リソースデータ(_backBuffers)にアクセスするためのレンダーターゲットビューをhandleアドレスに作成
	(
		renderingBuff.Get(),//レンダーターゲットを表す ID3D12Resource オブジェクトへのポインター
		&rtvDesc,//レンダー ターゲット ビューを記述する D3D12_RENDER_TARGET_VIEW_DESC 構造体へのポインター。
		rtvHeap->GetCPUDescriptorHandleForHeapStart()//新しく作成されたレンダーターゲットビューが存在する宛先を表す CPU 記述子ハンドル(ヒープ上のアドレス)
	);

	return S_OK;
}

HRESULT ResourceManager::CreateAndMapMatrix()
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
	auto angle = XMMatrixRotationY(3.14f);
	//worldMat *= angle; // モデルが後ろ向きなので180°回転して調整

	//ビュー行列の生成・乗算
	XMFLOAT3 eye(0, 5, 20);
	XMFLOAT3 target(0, 10, 0);
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
		300.0 // ファークリップ
	);

	matrixBuff->Map(0, nullptr, (void**)&mappedMatrix); // mapping
	mappedMatrix->world = worldMat;
	mappedMatrix->view = viewMat;
	mappedMatrix->proj = projMat;

	// Mapping Phong Material Parameters
	auto phongInfos = _fbxInfoManager->GetPhongMaterialParamertInfo();
	auto phongHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto phongResdesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(PhongInfo)/* * phongInfos.size()*/ + 0xff) & ~0xff);

	result = _dev->CreateCommittedResource
	(
		&phongHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&phongResdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
		nullptr,
		IID_PPV_ARGS(materialParamBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	materialParamBuff->Map(0, nullptr, (void**)&mappedPhong);
	//for (auto& info : phongInfos)
	//{
	//	mappedPhong->diffuse[0] = info.second.diffuse[0];
	//	mappedPhong->diffuse[1] = info.second.diffuse[1];
	//	mappedPhong->diffuse[2] = info.second.diffuse[2];

	//	mappedPhong->ambient[0] = info.second.ambient[0];
	//	mappedPhong->ambient[1] = info.second.ambient[1];
	//	mappedPhong->ambient[2] = info.second.ambient[2];

	//	mappedPhong->emissive[0] = info.second.emissive[0];
	//	mappedPhong->emissive[1] = info.second.emissive[1];
	//	mappedPhong->emissive[2] = info.second.emissive[2];

	//	mappedPhong->bump[0] = info.second.bump[0];
	//	mappedPhong->bump[1] = info.second.bump[1];
	//	mappedPhong->bump[2] = info.second.bump[2];

	//	mappedPhong->specular[0] = info.second.specular[0];
	//	mappedPhong->specular[1] = info.second.specular[1];
	//	mappedPhong->specular[2] = info.second.specular[2];

	//	mappedPhong->reflection[0] = info.second.reflection[0];
	//	mappedPhong->reflection[1] = info.second.reflection[1];
	//	mappedPhong->reflection[2] = info.second.reflection[2];

	//	mappedPhong->shineness = info.second.shineness;

	//	++mappedPhong;
	//}

	// create view
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = matrixBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = matrixBuff->GetDesc().Width;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; // SRV用ディスクリプタヒープ
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 31; // 1:Matrix(world, view, proj), 2:rendering result, 3:material paramerers * 29, 
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

	// 3:Phong Material Parameters

	for (int i = 0; i < phongInfos.size(); ++i)
	{
		handle.ptr += inc * (1 + i);
		D3D12_CONSTANT_BUFFER_VIEW_DESC phongCBVDesc = {};
		phongCBVDesc.BufferLocation = materialParamBuff->GetGPUVirtualAddress();
		phongCBVDesc.SizeInBytes = materialParamBuff->GetDesc().Width;
		_dev->CreateConstantBufferView
		(
			&phongCBVDesc,
			handle
		);
	}

	return S_OK;
}

void ResourceManager::ClearReference()
{
	_dev->Release();
	delete _prepareRenderingWindow;
}
#include <stdafx.h>
#include <ResourceManager.h>

ResourceManager::ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, PrepareRenderingWindow* prepareRenderingWindow) : _dev(dev), _fbxInfoManager(fbxInfoManager), _prepareRenderingWindow(prepareRenderingWindow){}

HRESULT ResourceManager::Init()
{
	HRESULT result = E_FAIL;

	// vertex Resource		
	auto vertMap = _fbxInfoManager->/*GetVertexMap*/GetIndexAndVertexPosByMeshName();
	auto itFirst = vertMap.begin();
	// create pos container
	for (int i = 0; i < vertMap.size(); ++i)
	{		
		auto itSecond = itFirst->second.begin();

		for (int j = 0; j < itFirst->second.size(); ++j)
		{
			verticesPosContainer.push_back(itSecond->second[0]);
			verticesPosContainer.push_back(itSecond->second[1]);
			verticesPosContainer.push_back(itSecond->second[2]);
			++itSecond;
		}
		++itFirst;
	}

	auto vertexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertresDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesPosContainer.size() * sizeof(/*VertexInfo*/float));

	result = _dev->CreateCommittedResource
	(
		&vertexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vertresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
		nullptr,
		IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	vertexTotalNum = verticesPosContainer.size() / 3;
	result = vertBuff->Map(0, nullptr, (void**)&mappedVertPos); // mapping
	std::copy(std::begin(verticesPosContainer), std::end(verticesPosContainer), mappedVertPos);
	vertBuff->Unmap(0, nullptr);

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//�o�b�t�@�̉��z�A�h���X
	vbView.SizeInBytes = vertresDesc.Width;//�S�o�C�g��
	vbView.StrideInBytes = 12;//1���_������̃o�C�g�� x,y,z

	// index Resource
	auto indiceMap = _fbxInfoManager->GetIndiceContainer();
	auto itIndiceFirst = indiceMap.begin();
	// create pos container
	for (int i = 0; i < indiceMap.size(); ++i)
	{
		auto itSecond = itIndiceFirst->second.begin();
		for (int j = 0; j < itIndiceFirst->second.size(); ++j)
		{
			indexContainer.push_back(itSecond[0]);
			++itSecond;
		}
		++itIndiceFirst;
	}

	indexNum = indexContainer.size();
	auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(_fbxInfoManager->GetIndexNum() * sizeof(int));
	result = _dev->CreateCommittedResource
	(
		&vertexHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&indicesDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
		nullptr,
		IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	result = idxBuff->Map(0, nullptr, (void**)&mappedIdx); // mapping
	std::copy(std::begin(indexContainer), std::end(indexContainer), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.SizeInBytes = indicesDesc.Width;
	ibView.Format = DXGI_FORMAT_R32_UINT;

	// depth DHeap, Resource
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1; // �[�x + lightmap + AO�p
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
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS; // �[�x�l�������ݗp
	depthResDesc.SampleDesc.Count = 1; // 1pixce/1�̃T���v��
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

	// �[�x�}�b�v�p
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

	//ClearReference();
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
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV�p�f�B�X�N���v�^�q�[�v
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK) return result;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	_dev->CreateRenderTargetView//���\�[�X�f�[�^(_backBuffers)�ɃA�N�Z�X���邽�߂̃����_�[�^�[�Q�b�g�r���[��handle�A�h���X�ɍ쐬
	(
		renderingBuff.Get(),//�����_�[�^�[�Q�b�g��\�� ID3D12Resource �I�u�W�F�N�g�ւ̃|�C���^�[
		&rtvDesc,//�����_�[ �^�[�Q�b�g �r���[���L�q���� D3D12_RENDER_TARGET_VIEW_DESC �\���̂ւ̃|�C���^�[�B
		rtvHeap->GetCPUDescriptorHandleForHeapStart()//�V�����쐬���ꂽ�����_�[�^�[�Q�b�g�r���[�����݂��鈶���\�� CPU �L�q�q�n���h��(�q�[�v��̃A�h���X)
	);

	return S_OK;
}

HRESULT ResourceManager::CreateAndMapMatrix()
{
	auto wvpHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto wvpResdesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(FBXSceneMatrix) + 0xff) & ~0xff);

	auto result = _dev->CreateCommittedResource
	(
		&wvpHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&wvpResdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
		nullptr,
		IID_PPV_ARGS(matrixBuff.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;

	//�s��p�萔�o�b�t�@�[�̐���
	auto worldMat = XMMatrixIdentity();

	//auto worldMat = XMMatrixRotationY(15.0f);
	auto angle = 0.0f;

	//�r���[�s��̐����E��Z
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH
	(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);

	//�v���W�F�N�V����(�ˉe)�s��̐����E��Z
	auto projMat = XMMatrixPerspectiveFovLH
	(
		XM_PIDIV2, // ��p90��
		static_cast<float>(_prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(_prepareRenderingWindow->GetWindowWidth()),
		1.0, // �j�A�\�N���b�v
		100.0 // �t�@�[�N���b�v
	);

	matrixBuff->Map(0, nullptr, (void**)&mappedMatrix); // mapping
	mappedMatrix->world = worldMat;
	mappedMatrix->view = viewMat;
	mappedMatrix->proj = projMat;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = matrixBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = matrixBuff->GetDesc().Width;

	// create view
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {}; // SRV�p�f�B�X�N���v�^�q�[�v
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 2; // 1:Matrix(world, view, proj)
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

	return S_OK;
}

void ResourceManager::ClearReference()
{
	_dev->Release();
	delete _prepareRenderingWindow;
}
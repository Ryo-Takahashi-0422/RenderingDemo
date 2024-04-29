#include <stdafx.h>
#include <D3DX12Wrapper.h>

#ifdef _DEBUG
#pragma comment(lib, "DirectXTexDebug.lib")
#else
#pragma comment(lib, "DirectXTexRelease.lib")
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// effekseer
//#pragma comment(lib, "EffekseerRendererDx12.lib")
//#pragma comment(lib, "Effekseer.lib")
//#pragma comment(lib, "LLGI.lib")
//#pragma comment(lib, "EffekseerRendererCommon.lib")
//#pragma comment(lib, "EffekseerRendererLLGI.lib")


using namespace DirectX;
using namespace Microsoft::WRL;

using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;
D3DX12Wrapper* D3DX12Wrapper::instance = nullptr;

D3DX12Wrapper::D3DX12Wrapper()
{
	instance = this;
};

void D3DX12Wrapper::DeleteInstance()
{
	instance = nullptr;

#ifdef _DEBUG
	_debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	//_debugDevice->Release();
	//_debugDevice = nullptr;
#endif
}

D3DX12Wrapper::~D3DX12Wrapper()
{

};

HRESULT D3DX12Wrapper::D3DX12DeviceInit()
{
	//result = CoInitializeEx(0, COINIT_MULTITHREADED);

	//�t�@�N�g���[�̐���
	result = S_OK;
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()))))
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()))))
		{
			return -1;
		}
	}

	//�O���{�������}������Ă���ꍇ�ɃA�_�v�^�[��I�����邽�߂̏���
	//�A�_�v�^�[�̗񋓗p
	std::vector <IDXGIAdapter*> adapters;
	//����̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, tmpAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		adapters.push_back(tmpAdapter.Get());
	}

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); //�A�_�v�^�[�̐����I�u�W�F�N�g�擾

		//�T�������A�_�v�^�[�̖��O���m�F
		std::wstring strDesc = adesc.Description;
		//printf("%ls\n", strDesc.c_str());
		//printf("%x\n", adesc.DeviceId);

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//�t�B�[�`�����x����
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Direct3D �f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter.Get(), lv, IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel = lv;
			break;//�����\�ȃo�[�W���������������烋�[�v���f
		}
	}
#ifdef _DEBUG
	_dev->QueryInterface(_debugDevice.GetAddressOf());
#endif

	_fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence./*ReleaseAnd*/GetAddressOf()));
}

//#ifdef _DEBUG
bool D3DX12Wrapper::PrepareRendering() {
//#else
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
//{
//#endif

	// SetRootSignatureBase�N���X�̃C���X�^���X��
	setRootSignature = new SetRootSignature;	

	// SettingShaderCompile�N���X�̃C���X�^���X��
	settingShaderCompile = new SettingShaderCompile;

	// VertexInputLayout�N���X�̃C���X�^���X��
	vertexInputLayout = new VertexInputLayout;

	// GraphicsPipelineSetting�N���X�̃C���X�^���X��
	gPLSetting = new GraphicsPipelineSetting(vertexInputLayout);
	vertexInputLayout = nullptr;

	// �����_�����O�E�B���h�E�ݒ�
	prepareRenderingWindow = new PrepareRenderingWindow();
	prepareRenderingWindow->CreateAppWindow(prepareRenderingWindow);

	// TextureLoader�N���X�̃C���X�^���X��
	//textureLoader = new TextureLoader;

	//// �����_�����O�E�B���h�E�\��
	//ShowWindow(prepareRenderingWindow->GetHWND(), SW_SHOW);

	// �r���[�|�[�g�ƃV�U�[�̈�̐ݒ�
	prepareRenderingWindow->SetViewportAndRect();

	// �L�[�{�[�h���͊Ǘ��N���X
	input = new Input(prepareRenderingWindow);

	//// ����߽�֘A�׽�Q
	peraLayout = new PeraLayout;
	peraGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/); //TODO PeraLayout,VertexInputLayout�׽�̊��N���X������Ă���ɑΉ�������
	peraPolygon = new PeraPolygon;
	peraSetRootSignature = new PeraSetRootSignature;
	peraShaderCompile = new SettingShaderCompile;

	peraLayout = nullptr;

	////�f�o�C�X�擾
	//auto hdc = GetDC(prepareRenderingWindow->GetHWND());
	//auto rate = GetDeviceCaps(hdc, VREFRESH);

	return true;
}

bool D3DX12Wrapper::PipelineInit(){
//���p�C�v���C���������@�����P�`�V
//�����������P�F�f�o�b�O���C���[���I����
#ifdef _DEBUG
	Utility::EnableDebugLayer();
#endif
//�����������Q�F�e��f�o�C�X�̏����ݒ�
	D3DX12DeviceInit();

	// �J���[�N���A�Ɋւ���Warning���t�B���^�����O(���b�Z�[�W��������Ă��܂�...)
	_dev.As(&infoQueue);

	D3D12_MESSAGE_ID denyIds[] = 
	{
	  D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE ,
	};
	D3D12_MESSAGE_SEVERITY severities[] = 
	{
	  D3D12_MESSAGE_SEVERITY_INFO
	};
	D3D12_INFO_QUEUE_FILTER filter{};
	filter.DenyList.NumIDs = _countof(denyIds);
	filter.DenyList.pIDList = denyIds;
	filter.DenyList.NumSeverities = _countof(severities);
	filter.DenyList.pSeverityList = severities;

	if (infoQueue != nullptr)
	{
		result = infoQueue->PushStorageFilter(&filter);
		// ���łɃG���[���b�Z�[�W�Ńu���[�N������
		result = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	}

//�����������R�F�R�}���h�L���[�̋L�q�p�ӁE�쐬

	//�R�}���h�L���[�����A�ڍ�obj����
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�^�C���A�E�g����
	cmdQueueDesc.NodeMask = 0;//�A�_�v�^�[��������g��Ȃ��Ƃ���0��OK
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�R�}���h�L���[�̗D��x
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�R�}���h���X�g�ƍ��킹��

	//�R�}���h�L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

//�����������S�F�X���b�v�`�F�[���̐���
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = prepareRenderingWindow->GetWindowWidth()/*1200.0f*/; // ����������
	swapChainDesc.Height = prepareRenderingWindow->GetWindowHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd( //�����Ő���
		_cmdQueue.Get(),
		prepareRenderingWindow->GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapChain./*ReleaseAnd*/GetAddressOf());
	if (result != S_OK) return false;

//�����������T�F�����_�[�^�[�Q�b�g�r���[(RTV)�̋L�q�q�q�[�v���쐬
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

//�����������U�F�t���[�����\�[�X(�e�t���[���̃����_�[�^�[�Q�b�g�r���[)���쐬
	_backBuffers.resize(swapChainDesc.BufferCount); // �ܯ�������ޯ��ޯ̧���ػ���
	handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//�q�[�v�̐擪��\�� CPU �L�q�q�n���h�����擾

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int idx = 0; idx < swapChainDesc.BufferCount; idx++)
	{   //swapEffect DXGI_SWAP_EFFECT_FLIP_DISCARD �̏ꍇ�͍ŏ��̃o�b�t�@�[�̂݃A�N�Z�X�\
		result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(_backBuffers[idx].ReleaseAndGetAddressOf()));//SWC�Ƀo�b�t�@�[��IID�Ƃ���IID�|�C���^��������(SWC�������_�����O���ɃA�N�Z�X����)
		if (result != S_OK) return false;

		_dev->CreateRenderTargetView//���\�[�X�f�[�^(_backBuffers)�ɃA�N�Z�X���邽�߂̃����_�[�^�[�Q�b�g�r���[��handle�A�h���X�ɍ쐬
		(
			_backBuffers[idx].Get(),//�����_�[�^�[�Q�b�g��\�� ID3D12Resource �I�u�W�F�N�g�ւ̃|�C���^�[
			&rtvDesc,//�����_�[ �^�[�Q�b�g �r���[���L�q���� D3D12_RENDER_TARGET_VIEW_DESC �\���̂ւ̃|�C���^�[�B
			handle//�V�����쐬���ꂽ�����_�[�^�[�Q�b�g�r���[�����݂��鈶���\�� CPU �L�q�q�n���h��(�q�[�v��̃A�h���X)
		);

		//handle���ڽ���L�q�q�̃A�h���X�������L�q�q�T�C�Y���I�t�Z�b�g���Ă���
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

//�����������V�F�R�}���h�A���P�[�^�[���쐬
			//�R�}���h�A���P�[�^�[����>>�R�}���h���X�g�쐬
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator2.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator3.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

	return true;
}

//void D3DX12Wrapper::EffekseerInit()
//{
//	_efkManager = Effekseer::Manager::Create(8000);
//	// DirectX�͍���n�̂��߁A����ɍ��킹��
//	_efkManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
//
//	auto graphicsDevice = EffekseerRendererDX12::CreateGraphicsDevice(_dev.Get(), _cmdQueue.Get(), 2);
//	
//	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	_efkRenderer = EffekseerRendererDX12::Create(graphicsDevice, &format, 1, DXGI_FORMAT_UNKNOWN, false, 8000);
//	_efkMemoryPool = EffekseerRenderer::CreateSingleFrameMemoryPool(_efkRenderer->GetGraphicsDevice());
//	_efkCmdList = EffekseerRenderer::CreateCommandList(_efkRenderer->GetGraphicsDevice(), _efkMemoryPool);
//
//	_efkRenderer->SetCommandList(_efkCmdList);
//
//	// �`�惂�W���[���̐ݒ�
//	_efkManager->SetSpriteRenderer(_efkRenderer->CreateSpriteRenderer());
//	_efkManager->SetRibbonRenderer(_efkRenderer->CreateRibbonRenderer());
//	_efkManager->SetRingRenderer(_efkRenderer->CreateRingRenderer());
//	_efkManager->SetTrackRenderer(_efkRenderer->CreateTrackRenderer());
//	_efkManager->SetModelRenderer(_efkRenderer->CreateModelRenderer());
//
//	// �e�N�X�`���A���f���A�J�[�u�A�}�e���A�����[�_�[�̐ݒ肷��B
//	// ���[�U�[���Ǝ��Ŋg���ł���B���݂̓t�@�C������ǂݍ���ł���B
//	_efkManager->SetTextureLoader(_efkRenderer->CreateTextureLoader());
//	_efkManager->SetModelLoader(_efkRenderer->CreateModelLoader());
//	_efkManager->SetMaterialLoader(_efkRenderer->CreateMaterialLoader());
//	_efkManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());
//
//	// �G�t�F�N�g���̂̐ݒ�
//	_effect = Effekseer::Effect::Create
//	(
//		_efkManager,
//		(const EFK_CHAR*)L"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\EffekseerTexture\\10\\SimpleLaser.efk",
//		1.0f,
//		(const EFK_CHAR*)L"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\EffekseerTexture\\10"
//	);
//	/*_efkHandle = _efkManager->Play(_effect, 0, 0, 0);*/
//}

bool D3DX12Wrapper::ResourceInit() {
	//�����\�[�X������

	// �[�����̃��f���p�X���擾���Ċi�[����
	auto path = Utility::GetModelFilepath();
	auto path_Sponza = path + "\\Sponza.bin";
	auto path_Connan = path + "\\Connan.bin";
	modelPath.push_back(path_Sponza);
	modelPath.push_back(path_Connan);
	
	resourceManager.resize(modelPath.size());
	FBXInfoManager fbxInfoManager;

	camera = Camera::GetInstance();
	camera->Init(prepareRenderingWindow);

	for (int i = 0; i < modelPath.size(); ++i)
	{
		// FBXInfoManager Instance
		fbxInfoManager = FBXInfoManager::Instance();
		fbxInfoManager.Init(modelPath[i]);

		if (i == 0)
		{
			occManager = new OcclusionCullingManager(_dev, &fbxInfoManager, 1024, 1024);
		}
		
		// FBX resource creation
		resourceManager[i] = new ResourceManager(_dev, &fbxInfoManager, prepareRenderingWindow);		
		resourceManager[i]->Init(camera);		
	}	
	
	// TextureTransporter�N���X�̃C���X�^���X��
	textureTransporter = new TextureTransporter;
	
	// ����������1�F���[�g�V�O�l�`���ݒ�
	if (FAILED(setRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}

	// ����߽�p
	if (FAILED(peraSetRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}
	
// ����������2�F�V�F�[�_�[�R���p�C���ݒ�
    
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _vsMBlob = nullptr; // ����߽�p���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psMBlob = nullptr; // ����߽�p���_�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _vsBackbufferBlob = nullptr; // �\���p���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBackbufferBlob = nullptr; // �\���p���_�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p

	// FBX���f���`��p�̃V�F�[�_�[�Z�b�e�B���O
	std::string fbxVs = "FBXVertex.hlsl";
	std::string fbxPs = "FBXPixel.hlsl";
	auto fbxPathPair = Utility::GetHlslFilepath(fbxVs, fbxPs);
	auto blobs = settingShaderCompile->SetShaderCompile(setRootSignature, _vsBlob, _psBlob, 
		fbxPathPair.first, "FBXVS",
		fbxPathPair.second, "FBXPS");
	if (blobs.first == nullptr or blobs.second == nullptr) return false;
	_vsBlob = blobs.first;
	_psBlob = blobs.second;	

	delete settingShaderCompile;
	settingShaderCompile = nullptr;

	// �o�b�N�o�b�t�@�`��p
	std::string bufferVs = "PeraVertex.hlsl"; // PreFxaa.cpp�Ƌ���
	std::string bufferPs = "FXAAOutputPixel.hlsl";
	auto bufferPathPair = Utility::GetHlslFilepath(bufferVs, bufferPs);
	auto mBlobs = peraShaderCompile->PeraSetShaderCompile(peraSetRootSignature, _vsMBlob, _psMBlob,
		bufferPathPair.first, "vs",
		bufferPathPair.second, "ps_main");
	if (mBlobs.first == nullptr or mBlobs.second == nullptr) return false;
	_vsMBlob = mBlobs.first;
	_psMBlob = mBlobs.second;

	delete peraShaderCompile;
	peraShaderCompile = nullptr;

// ����������3�F���_���̓��C�A�E�g�̍쐬�y��
// ����������4�F�p�C�v���C����ԃI�u�W�F�N�g(PSO)��Desc�L�q���ăI�u�W�F�N�g�쐬
	result = gPLSetting->CreateGPStateWrapper(_dev, setRootSignature, _vsBlob, _psBlob);

	// �ޯ��ޯ̧�p
	result = peraGPLSetting->CreateGPStateWrapper(_dev, peraSetRootSignature, _vsMBlob, _psMBlob);

// ����������5�F�R�}���h���X�g����
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator2.Get(), nullptr, IID_PPV_ARGS(_cmdList2.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator3.Get(), nullptr, IID_PPV_ARGS(_cmdList3.ReleaseAndGetAddressOf()));
	m_batchSubmit[0] = _cmdList.Get();
	m_batchSubmit[1] = _cmdList2.Get();
// ����������6�F�R�}���h���X�g�̃N���[�Y(�R�}���h���X�g�̎��s�O�ɂ͕K���N���[�Y����)

// ����������7�F�e�o�b�t�@�[���쐬���Ē��_����ǂݍ���

	
	//�t�@�C���`�����̃e�N�X�`�����[�h����
	//textureLoader->LoadTexture();

	// �����Ƀ~�b�v�}�b�v�쐬������ǉ�


	// �e�N�X�`���A�b�v���[�h
	for (int i = 0; i < modelPath.size(); ++i)
	{
		if (resourceManager[i]->GetMaterialAndTexturePath().size() != 0)
		{
			textureTransporter->TransportPMDMaterialTexture(resourceManager[i], _cmdList, _cmdAllocator, _cmdQueue,
				resourceManager[i]->GetTextureMetaData(), resourceManager[i]->GetTextureImg(),
				_fence, _fenceVal, resourceManager[i]->GetTextureUploadBuff(), resourceManager[i]->GetTextureReadBuff());
		}
	}
	//textureLoader = nullptr;
	delete textureTransporter;
	textureTransporter = nullptr;

	// �}���`�p�X�p�r���[�쐬
	peraPolygon->CreatePeraView(_dev);

// ����������9�F�t�F���X�̐���
// ����������10�F�C�x���g�n���h���̍쐬
// ����������11�FGPU�̏��������҂�
// Imgui�Ǝ��̏����ݒ�
	settingImgui = new SettingImgui;

	if (FAILED(settingImgui->Init(_dev, prepareRenderingWindow)))
	{
		return false;
	}	

	XMFLOAT3 p1 = XMFLOAT3(25.0f, 15.0f, 0.0f);
	XMFLOAT3 p2 = XMFLOAT3(-25.0f, 15.0f, 0.0f);
	settingImgui->SetInitialLightPos(p1, p2);
	resourceManager[0]->SetLightPos1(p1);
	resourceManager[1]->SetLightPos1(p1);
	resourceManager[0]->SetLightPos2(p2);
	resourceManager[1]->SetLightPos2(p2);

//// DirectXTK�Ǝ��̏����ݒ�
//	DirectXTKInit();
//	

	auto initialWidth = prepareRenderingWindow->GetWindowWidth();
	auto initialHeight = prepareRenderingWindow->GetWindowHeight();

	for (auto& reManager : resourceManager)
	{
		reManager->ClearReference();
	}

	viewPort = prepareRenderingWindow->GetViewPortPointer();
	rect = prepareRenderingWindow->GetRectPointer();
	changeableViewport = prepareRenderingWindow->GetChangeableViewPortPointer();
	changeableRect = prepareRenderingWindow->GetChangeableRectPointer();

	// Sky�ݒ�
	calculatedParticipatingMedia = participatingMedia.calculateUnit();

	sun = new Sun(_dev.Get(), camera, initialWidth, initialHeight);
	sun->Init();
	
	shadowFactor = new ShadowFactor(_dev.Get(), _fence.Get(), initialWidth, initialHeight);
	shadowFactor->SetParticipatingMedia(calculatedParticipatingMedia);
	auto shadowFactorResource = shadowFactor->GetShadowFactorTextureResource();

	skyLUT = new SkyLUT(_dev.Get(), shadowFactorResource.Get(), initialWidth, initialHeight);
	skyLUT->SetParticipatingMedia(calculatedParticipatingMedia);
	skyLUTBuffer.eyePos.x = camera->GetWorld().r[3].m128_f32[0];
	skyLUTBuffer.eyePos.y = camera->GetWorld().r[3].m128_f32[1];
	skyLUTBuffer.eyePos.z = camera->GetWorld().r[3].m128_f32[2];
	skyLUTBuffer.stepCnt = 32;
	skyLUTBuffer.sunDirection.x = sun->GetDirection().x;
	skyLUTBuffer.sunDirection.y = sun->GetDirection().y;
	skyLUTBuffer.sunDirection.z = sun->GetDirection().z;
	skyLUTBuffer.sunIntensity.x = 1.0f;
	skyLUTBuffer.sunIntensity.y = 1.0f;
	skyLUTBuffer.sunIntensity.z = 1.0f;
	skyLUT->SetSkyLUTBuffer(skyLUTBuffer);
	skyLUT->SetSkyLUTResolution();

	auto skyLUTResource = skyLUT->GetSkyLUTRenderingResource();
	sky = new Sky(_dev.Get(), _fence.Get(), skyLUTResource.Get());
	sky->SetSceneInfo(camera->GetWorld());
	
	camera->CalculateFrustum();
	camera->SetDummyFrustum();
	camera->MoveCamera(0, XMMatrixIdentity()); // camera view�s�񏉊����@������Α��z�̏����ʒu�ɉe������
	camera->CalculateOribitView(XMFLOAT3(0, 1.5, 0), connanDirection); // orbitview�̏������B�������Ȃ��ꍇ�A�N������air���������`�悳��Ȃ��B

	sky->SetFrustum(camera->GetFrustum());

	sun->SetShadowFactorResource(shadowFactorResource.Get());

	shadow = new Shadow(_dev.Get(), 4096, 4096);
	shadow->Init();
	
	air = new Air(_dev.Get(), _fence.Get(), shadow->GetShadowMapResource(), shadowFactor->GetShadowFactorTextureResource());
	air->SetFrustum(camera->GetFrustum());
	calculatedParticipatingMedia.mieScattering = 7.0f * 1e-6f;
	calculatedParticipatingMedia.asymmetryParameter = 0.0f;
	calculatedParticipatingMedia.altitudeOfRayleigh = 0.0f;
	calculatedParticipatingMedia.altitudeOfMie = 3500.0f;
	air->SetParticipatingMedia(calculatedParticipatingMedia);
	
	// vsm blur
	shadowRenderingBlur = new Blur(_dev.Get());
	std::string vs = "BlurVertex.hlsl";
	std::string ps = "BlurPixel.hlsl";
	auto pair = Utility::GetHlslFilepath(vs, ps);
	shadowRenderingBlur->Init(pair, shadow->GetResolution());
	shadowRenderingBlur->SetRenderingResourse(shadow->GetShadowRenderingResource());
	shadowRenderingBlur->SetSwitch(true);
	
	// resourceManager[0]�݂̂Ɋi�[...
	resourceManager[0]->SetSunResourceAndCreateView(sun->GetRenderResource());
	resourceManager[0]->SetSkyResourceAndCreateView(sky->GetSkyLUTRenderingResource());
	resourceManager[0]->SetImGuiResourceAndCreateView(settingImgui->GetImguiRenderingResource());
	// air(�{�����[�����C�e�B���O)�̓I�u�W�F�N�g�ƃL�����N�^�[�ŗ��p
	resourceManager[0]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	resourceManager[1]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	// VSM���Z�b�g
	resourceManager[0]->SetVSMResourceAndCreateView(shadowRenderingBlur->GetBlurResource());
	resourceManager[1]->SetVSMResourceAndCreateView(shadowRenderingBlur->GetBlurResource());
	// air�̃u���[���ʂ��Z�b�g
	//resourceManager[0]->SetVSMResourceAndCreateView(airBlur->GetBlurResource());
	//resourceManager[1]->SetVSMResourceAndCreateView(airBlur->GetBlurResource());
	// �u���[�����V���h�E�}�b�v���Z�b�g
	//resourceManager[0]->SetShadowResourceAndCreateView(/*shadow->GetShadowMapResource()*/comBlur->GetBlurTextureResource());
	//resourceManager[1]->SetShadowResourceAndCreateView(/*shadow->GetShadowMapResource()*/comBlur->GetBlurTextureResource());
	// ���s���e�r���[�𗘗p����vsm�ŉe��`�悷��ꍇ�ɗ��p����s���sun���擾����
	resourceManager[0]->SetProjMatrix(sun->GetProjMatrix());
	resourceManager[1]->SetProjMatrix(sun->GetProjMatrix());

	// �摜�����N���X�̏����t���[
	// integration��thread1,2�̓����J���[�A�@���ۗL��ԂɂȂ�
	// depthMapIntegration��thread1,2�̃f�v�X�}�b�v����
	// ��comBlur�ŃK�E�V�A���u���[
	// ��integration��thread1,2�̓����J���[�A�@���A�f�v�X�}�b�v�ۗL��ԂɂȂ�
	integration = new Integration(_dev.Get(), resourceManager[0]->GetSRVHeap(), initialWidth, initialHeight);
	depthMapIntegration = new DepthMapIntegration(_dev.Get(), resourceManager[0]->GetDepthBuff(), resourceManager[0]->GetDepthBuff2(), initialWidth, initialHeight);
	//comBlur = new ComputeBlur(_dev.Get(), depthMapIntegration->GetTextureResource());
	calculateSSAO = new CalculateSSAO(_dev.Get(), integration->GetNormalResourse(), /*comBlur->GetBlurTextureResource()*/depthMapIntegration->GetTextureResource(), initialWidth, initialHeight); // �u���[��������SSAO�v�Z���Ă����܂�ς��Ȃ�...
	calculateSSAO->SetType(settingImgui->GetSSAOBoxCheck(), settingImgui->GetRTAOBoxCheck());
	ssaoBlur = new Blur(_dev.Get());
	ssaoBlur->Init(pair, calculateSSAO->GetResolution());
	ssaoBlur->SetRenderingResourse(calculateSSAO->GetTextureResource());
	ssaoBlur->SetSwitch(true);
	integration->SetResourse1(/*calculateSSAO->GetTextureResource()*/ssaoBlur->GetBlurResource());

	// integrated color blur for ��ʊE�[�x
	colorIntegraredBlur = new Blur(_dev.Get());
	colorIntegraredBlur->Init(pair, integration->GetResolution());
	colorIntegraredBlur->SetRenderingResourse(integration->GetColorResourse());
	colorIntegraredBlur->SetSwitch(true);
	integration->SetResourse2(colorIntegraredBlur->GetBlurResource());
	integration->SetResourse3(depthMapIntegration->GetTextureResource());

	preFxaa = new PreFxaa(_dev.Get(), initialWidth, initialHeight);
	preFxaa->SetResourse1(integration->GetColorResourse());
	preFxaa->SetResourse2(integration->GetImguiResourse());
	preFxaa->SetResourse3(ssaoBlur->GetBlurResource());
	preFxaa->SetResourse4(colorIntegraredBlur->GetBlurResource());
	preFxaa->SetResourse5(depthMapIntegration->GetTextureResource());
	preFxaa->SetExternalView();
	preFxaa->SetInitialInfos();

	return true;
}

void D3DX12Wrapper::SetMatrixByFPSChange(int fpsVal)
{
	// ������
	forwardSpeed = -0.026;
	leftSpinMatrix = XMMatrixRotationY(-turnSpeed);
	rightSpinMatrix = XMMatrixInverse(nullptr, leftSpinMatrix);

	auto _fps = fpsVal;
	float ratio = 120.0f / _fps;
	forwardSpeed *= ratio;
	MIN_FREAM_TIME = 1.0f / _fps;


	XMMATRIX origin = XMMatrixIdentity();
	origin.r[0].m128_f32[0] = 0.99982235237812678;
	origin.r[0].m128_f32[1] = 0.0f;
	origin.r[0].m128_f32[2] = -0.018848439857690965;

	origin.r[1].m128_f32[0] = 0.0f;
	origin.r[1].m128_f32[1] = 1.0f;
	origin.r[1].m128_f32[2] = 0.0f;

	origin.r[2].m128_f32[0] = 0.018848439857690965;
	origin.r[2].m128_f32[1] = 0.0f;
	origin.r[2].m128_f32[2] = 0.99982235237812678;

	origin *= ratio;

	leftSpinMatrix = XMMatrixRotationY(-PI * 0.006f * ratio);
	rightSpinMatrix = XMMatrixRotationY(PI * 0.006f * ratio);
}

void D3DX12Wrapper::Run() {
	MSG msg = {};
	cbv_srv_Size = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//// �G�t�F�N�g�̍Đ�
	//_efkHandle = _efkManager->Play(_effect, 0, 0, 0);

	// ���[�V�����ݒ�
	for (int i = 0; i < modelPath.size(); ++i)
	{
		if (resourceManager[i]->GetIsAnimationModel())
		{
			resourceManager[i]->PlayAnimation();
			std::vector<std::string> motionNames;
			auto motionData = resourceManager[i]->GetAnimationNameAndBoneNameWithTranslationMatrix();
			for (auto it = motionData.begin(); it!= motionData.end(); it++)
			{
				if (it->first == "Armature|Idle")
				{
					idleMotionDataNameAndMaxFrame.first = it->first;
					idleMotionDataNameAndMaxFrame.second = it->second[0].size();
				}

				else if (it->first == "Armature|Walking")
				{
					walkingMotionDataNameAndMaxFrame.first = it->first;
					walkingMotionDataNameAndMaxFrame.second = it->second[0].size();
				}

				else if (it->first == "Armature|Run")
				{
					runMotionDataNameAndMaxFrame.first = it->first;
					runMotionDataNameAndMaxFrame.second = it->second[0].size();
				}
			}

			resourceManager[i]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);
		}
	}

	// fps�ݒ�
	float fps = 0;
	float frameTime = 0;
	LARGE_INTEGER timeStart;
	LARGE_INTEGER timeEnd;
	LARGE_INTEGER timeFreq;

	// �Փ˔��菀��
	collisionManager = new CollisionManager(_dev, resourceManager);
	oBBManager = new OBBManager(_dev, resourceManager);

	// fps�ݒ���O�i�E��]���x�̌v�Z���s��
	auto fpsValue = settingImgui->GetFPS();
	SetMatrixByFPSChange(fpsValue);
	bool isFpsChanged = false;

	// ���C�����[�v�ɓ���O�ɐ��x���擾
	if (QueryPerformanceFrequency(&timeFreq) == FALSE) { // ���̊֐���0(FALSE)���A�鎞�͖��Ή�
		return;
	}
	// ����v�Z�p
	QueryPerformanceCounter(&timeStart);
	modelPathSize = modelPath.size();
	DWORD sleepTime;

	for (int fbxIndex = 0; fbxIndex < modelPathSize; ++fbxIndex) // �����[�u�Z�}���e�B�N�X�ɂ��|�C���^���L���̈ڍs��������Optimized C++ P215 ��vector��resourceManager��swap�Ń������J������
	{
		auto vbView = resourceManager[fbxIndex]->GetVbView();
		vbViews.push_back(vbView);

		auto ibView = resourceManager[fbxIndex]->GetIbView();
		ibViews.push_back(ibView);

		auto srvHeapAddress = resourceManager[fbxIndex]->GetSRVHeap();
		srvHeapAddresses.push_back(srvHeapAddress);

		auto dHandle = resourceManager[fbxIndex]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		dHandles.push_back(dHandle);

		auto container = resourceManager[fbxIndex]->GetIndiceAndVertexInfoOfRenderingMesh();
		indiceContainer.push_back(container);
		auto itIndice = indiceContainer[fbxIndex].begin();
		itIndiceFirsts.push_back(itIndice);

		auto phongInfo = resourceManager[fbxIndex]->GetPhongMaterialParamertInfo();
		phongInfos.push_back(phongInfo);
		auto itPhonsInfo = phongInfos[fbxIndex].begin();
		itPhonsInfos.push_back(itPhonsInfo);

		auto materialAndTexturename = resourceManager[fbxIndex]->GetMaterialAndTexturePath();
		materialAndTexturenameInfo.push_back(materialAndTexturename);
		auto itMaterialAndTextureName = materialAndTexturenameInfo[fbxIndex].begin();
		itMaterialAndTextureNames.push_back(itMaterialAndTextureName);
		int matTexSize = materialAndTexturenameInfo[fbxIndex].size();
		matTexSizes.push_back(matTexSize);
	}

	int index = 0;
	int num = 0;
	std::string lastMaterialName;
	for (auto& infos : materialAndTexturenameInfo)
	{
		for (int i = 0; i < infos.size(); ++i)
		{
			if (i > 0)
			{
				auto currentMaterialName = infos[i].first;

				if (currentMaterialName != lastMaterialName)
				{
					textureIndexes[index].push_back(num);
					num = 0;
				}
			}
			lastMaterialName = infos[i].first;
			num++;
		}

		textureIndexes[index].push_back(num);

		++index;
		num = 0;
	}

	// �e�̕`��ł����p����
	shadow->SetVertexAndIndexInfo(vbViews, ibViews, itIndiceFirsts, indiceContainer);

	HANDLE event; // fnece�p�C�x���g
	
	fBXPipeline = gPLSetting->GetPipelineState().Get();
	fBXRootsignature = setRootSignature->GetRootSignature().Get();

	// DrawBackBuffer�ŗ��p����
	bBRootsignature = peraSetRootSignature->GetRootSignature().Get();
	bBPipeline = peraGPLSetting->GetPipelineState().Get();

	ID3D12CommandList* m_cmdLists[2];
	int listNum = 0;
	for (auto& cList : m_batchSubmit)
	{
		m_cmdLists[listNum] = cList.Get();
	}

	LoadContexts();
	bbIdx = 0;

	XMFLOAT3 sunDir;
	shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get()); // �ȍ~�͉𑜓x�ɕύX������ꍇ�̂ݕ`�悷��
	sun->ChangeSceneMatrix(XMMatrixIdentity());

	while (true)
	{	
		isFpsChanged = settingImgui->GetIsFpsChanged();
		if (isFpsChanged)
		{
			fpsValue = settingImgui->GetFPS();
			SetMatrixByFPSChange(fpsValue);
			isFpsChanged = false;
		}

		// �����Ԃ��擾
		QueryPerformanceCounter(&timeEnd);
		// (������ - �O�t���[���̎���) / ���g�� = �o�ߎ���(�b�P��)
		frameTime = static_cast<float>(timeEnd.QuadPart - timeStart.QuadPart) / static_cast<float>(timeFreq.QuadPart);

		if (frameTime < MIN_FREAM_TIME) { // ���Ԃɗ]�T������ꍇ
		// �~���b�ɕϊ�
			sleepTime = static_cast<DWORD>((MIN_FREAM_TIME - frameTime) * 1000);
			timeBeginPeriod(1); // ����\���グ��(Sleep���x����)
			Sleep(sleepTime);
			timeEndPeriod(1);   // �߂�
			continue;
		}

		timeStart = timeEnd; // ����ւ�

		// �������𖳂����Ƃ��Ȃ�y���Ȃ�
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���I������message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			printf("%s", "quit");
			break;
		}

		// �E�B���h�E�T�C�Y�ύX���������ꍇ�̏���
		bool isWindowSizeChanged = prepareRenderingWindow->GetWindowSizeChanged();
		if (isWindowSizeChanged)
		{
			UINT clientWidth = prepareRenderingWindow->GetWindowWidth();
			UINT clientHeight = prepareRenderingWindow->GetWindowHeight();

			settingImgui->ChangeResolution(_dev, prepareRenderingWindow, clientWidth, clientHeight);
			resourceManager[0]->SetImGuiResourceAndCreateView(settingImgui->GetImguiRenderingResource());
		}
		settingImgui->DrawImGUI(_dev, _cmdList);

		// �|�C���g���C�g�̈ʒu�ɕύX������ꍇ
		bool isPointLightPosChanged = settingImgui->GetIsLightPosChanged();
		if (isPointLightPosChanged)
		{
			auto p1 = settingImgui->GetLightPos1();
			auto p2 = settingImgui->GetLightPos2();
			resourceManager[0]->SetLightPos1(p1);
			resourceManager[1]->SetLightPos1(p1);
			resourceManager[0]->SetLightPos2(p2);
			resourceManager[1]->SetLightPos2(p2);
		}


		// ���z�̈ʒu���X�V
		sunDir = sun->CalculateDirectionFromDegrees(settingImgui->GetSunAngleX(), settingImgui->GetSunAngleY());
		sun->CalculateViewMatrix();
		shadow->SetVPMatrix(sun->GetShadowViewMatrix(), sun->GetProjMatrix()); // sun->GetViewMatrix()
		shadow->SetSunPos(sun->GetFixedDirection());

		// VSM�𗘗p����ꍇ�͕��s���e�r���[�s��𗘗p���邽�߈ȉ���sunDir���������p����
		skyLUTBuffer.sunDirection.x = sunDir.x;
		skyLUTBuffer.sunDirection.y = sunDir.y;
		skyLUTBuffer.sunDirection.z = sunDir.z;

		// �[�x�}�b�v���g���ꍇ�͓������e�s��ɂ��r���[�s����쐬���Ă��邽�߁A���z�ʒu���J�����ʒu�ɍ��킹�Ē������Ă���B����ɔ����AskyLUT�ɂ����鑾�z�̈ʒu�����킹�Ē�������K�v������B
		//skyLUTBuffer.sunDirection = sun->GetPersedSunDirection();
		skyLUT->SetSkyLUTBuffer(skyLUTBuffer);

		air->SetFrustum(camera->GetDummyFrustum());
		air->SetSceneInfo(sun->GetShadowViewMatrix(), sun->GetProjMatrix(), camera->GetDummyCameraPos(), sun->GetDirection()); // sun->GetViewMatrix()

		// Shadow Factor�̉𑜓x�ύX�̓v���O�������N���b�V�����邽�߈ꎞ����
		// ShadowFactor�̉𑜓x�ɕύX������ꍇ�̏���
		//if (settingImgui->GetIsShadowFactorResolutionChanged())
		//{
		//	shadowFactor->ChangeResolution(settingImgui->GetShadowFactorResX(), settingImgui->GetShadowFactorResY());
		//	skyLUT->SetShadowFactorResource(shadowFactor->GetShadowFactorTextureResource().Get());
		//	shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get());
		//	skyLUT->SetBarrierSWTrue();
		//}
		
		// Air��Participating Media�ɕύX������ꍇ�̏���
		if (settingImgui->GetIsAirParamChanged())
		{
			ParticipatingMedia* changedMedia = new ParticipatingMedia;
			changedMedia = settingImgui->GetParticipatingMediaParam();
			auto c = participatingMedia.SetAndcalculateUnit(changedMedia);
			air->SetParticipatingMedia(c);
		}

		//SkyLUT��Participating Media�ɕύX������ꍇ�̏���
		if (settingImgui->GetIsSkyLUTParamChanged())
		{
			ParticipatingMedia* changedMedia = new ParticipatingMedia;
			changedMedia = settingImgui->GetPMediaParam4SkyLUT();
			auto c = participatingMedia.SetAndcalculateUnit(changedMedia);
			skyLUT->SetParticipatingMedia(c);
		}

		// SkyLUT�̉𑜓x�ɕύX������ꍇ�̏���
		if (settingImgui->GetIsSkyLUTResolutionChanged())
		{
			skyLUT->ChangeSkyLUTResolution(settingImgui->GetSkyLUTResX(), settingImgui->GetSkyLUTResY());
			sky->ChangeSkyLUTResourceAndView(skyLUT->GetSkyLUTRenderingResource().Get());
		}

		// Sky�̉𑜓x�ɕύX������ꍇ�̏���
		if (settingImgui->GetIsSkyResolutionChanged())
		{
			sky->ChangeSceneResolution(settingImgui->GetSkyResX(), settingImgui->GetSkyResY());
			resourceManager[0]->SetSkyResourceAndCreateView(sky->GetSkyLUTRenderingResource());
		}
		
		// �J�����̓L�����N�^�[�ړ��ɒǏ]����B���z�̓��[���h���_(0,0,0)�����̊p�x�w��*100�̈ʒu�ɌŒ肳��Ă��邽�߁A���z�`�掞�̓r���{�[�h��Z���J�����ʒu(�Ǐ]�ʒu)�֕��s�ړ������z�����֕��s�ړ��Ƃ���B
		sun->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);

		shadow->SetBoneMatrix(resourceManager[1]->GetMappedMatrixPointer()); // �V���h�E�}�b�v�ł̃L�����N�^�[�A�j���[�V���������ɗ��p����
		shadow->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);

		bool airDraw = settingImgui->GetAirBoxChanged();
		if (airDraw)
		{
			air->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get()); // ��shadow�𗘗p
		}		
		skyLUT->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		sky->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), viewPort, rect);
		shadowRenderingBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect); // ��vsm shadow�𗘗p

		resourceManager[0]->SetSceneInfo(shadow->GetShadowPosMatrix(), shadow->GetShadowPosInvMatrix(), shadow->GetShadowView(), camera->GetDummyCameraPos(), sun->GetDirection());
		resourceManager[1]->SetSceneInfo(shadow->GetShadowPosMatrix(), shadow->GetShadowPosInvMatrix(), shadow->GetShadowView(), camera->GetDummyCameraPos(), sun->GetDirection());
		for (int i = 0; i < threadNum; i++)
		{
			SetEvent(m_workerBeginRenderFrame[i]);			
		}
		WaitForMultipleObjects(threadNum, m_workerFinishedRenderFrame, TRUE, INFINITE); // DrawBackBuffer�ɂ�����h���[�R�[�����O�ɒu���Ă�fps�͉��P����...

		// �摜����������SSAO����
		integration->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);
		colorIntegraredBlur->ChangeSwitch(settingImgui->GetDOFBoxChanged());
		colorIntegraredBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);
		depthMapIntegration->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		//comBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		auto proj = camera->GetProj();
		calculateSSAO->SetDraw(settingImgui->GetSSAOBoxChanged());
		calculateSSAO->SetInvVPMatrix(/*camera->GetOrbitView(), camera->GetInvView(), */proj, XMMatrixInverse(nullptr, proj));

		bool aoTypeChanged = settingImgui->GetAOTypeChanged();
		if (aoTypeChanged)
		{
			calculateSSAO->SetType(settingImgui->GetSSAOBoxCheck(), settingImgui->GetRTAOBoxCheck());
		}
		calculateSSAO->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		ssaoBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);

		// air�̃R�s�[�p���\�[�X��Ԃ�UAV�ɖ߂�
		if (airDraw)
		{
			auto barrierDescOfAirCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
			(
				resourceManager[0]->GetAirBuff().Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			);
			_cmdList3->ResourceBarrier(1, &barrierDescOfAirCopyDestTexture);
		}
		// �V���h�E�}�b�v��[�x�������݉\�ȏ�Ԃɖ߂�
		auto barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
		(
			shadow->GetShadowMapResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		);
		_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);

		// �f�v�X�}�b�v�����N���X �R�s�[�p���\�[�X��Ԃ�UAV�ɂ���
		auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		(
			depthMapIntegration->GetTextureResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

		AllKeyBoolFalse();
		// �e���f���̃f�u�X�}�b�v��ǂݍ��݉\��ԂɕύX����
		D3D12_RESOURCE_BARRIER barrierDesc4DepthMap2 = CD3DX12_RESOURCE_BARRIER::Transition
		(
			resourceManager[0]->GetDepthBuff().Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap2);

		if (settingImgui->GetIsFxaaChanged())
		{
			preFxaa->SetFxaaDraw(settingImgui->GetFxaaBoxBoolean());
		}
		preFxaa->Execution(_cmdList3.Get(), viewPort, rect);
		barrierDesc4DepthMap2.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierDesc4DepthMap2.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap2);
		DrawBackBuffer(cbv_srv_Size);

		// calculateSSAO ���p�I���ɂ��A�R�s�[�p���\�[�X��Ԃ�UAV�ɂ���
		barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		(
			calculateSSAO->GetTextureResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

		_cmdList3->Close();
		//_cmdList2->Close();
		
		//�R�}���h�L���[�̎��s
		ID3D12CommandList* cmdLists[] = { _cmdList.Get(), _cmdList2.Get(), _cmdList3.Get() };
		_cmdQueue->ExecuteCommandLists(3, cmdLists);

		//ID3D12Fence��Signal��CPU���̃t�F���X�ő������s
		//ID3D12CommandQueue��Signal��GPU���̃t�F���X��
		//�R�}���h�L���[�ɑ΂��鑼�̂��ׂĂ̑��삪����������Ƀt�F���X�X�V
		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

		while (_fence->GetCompletedValue() != _fenceVal)
		{			
			event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//�C�x���g�����҂�
			WaitForSingleObject(event, INFINITE);
			//�C�x���g�n���h�������
			CloseHandle(event);
		}

		_cmdAllocator->Reset();//�R�}���h �A���P�[�^�[�Ɋ֘A�t�����Ă��郁�������ė��p����
		_cmdList->Reset(_cmdAllocator.Get(), nullptr);

		_cmdAllocator2->Reset();//�R�}���h �A���P�[�^�[�Ɋ֘A�t�����Ă��郁�������ė��p����
		_cmdList2->Reset(_cmdAllocator2.Get(), nullptr);

		_cmdAllocator3->Reset();
		_cmdList3->Reset(_cmdAllocator3.Get(), nullptr);

		//�t���b�v���ă����_�����O���ꂽ�C���[�W��\��
		_swapChain->Present(1, 0);	
		bbIdx = _swapChain->GetCurrentBackBufferIndex();//���݂̃o�b�N�o�b�t�@���C���f�b�N�X�ɂĎ擾

		// shadow�𑜓x�̕ύX������ �R�[�h�\���I��shadow�`�撼�O�ɋL�q���������A��������Ɖ𑜓x�ύX���Ɉ�u�Ó]���邽�߃o�b�N�o�b�t�@�̃t���b�v��ɋL�q���Ă���B
		bool isShadowResChanged = settingImgui->GetIsShadowResolutionChanged();
		if (isShadowResChanged)
		{
			// shadow��sky�̂悤�ȉ𑜓x�ύX�����\�[�X���ς��̎�i�ɂ����fence�l���o�O�邽�߃C���X�^���X�V�K�����őΉ����Ă���
			shadow = nullptr;
			shadow = new Shadow(_dev.Get(), settingImgui->GetShadowResX(), settingImgui->GetShadowResY());
			shadow->Init();
			shadow->SetVertexAndIndexInfo(vbViews, ibViews, itIndiceFirsts, indiceContainer);
			shadow->SetMoveMatrix(resourceManager[1]->GetMappedMatrix()->world);
			shadow->SetRotationMatrix(connanDirection);
			// air�͒���ɂ���shadowRenderingBlur�̂悤�Ƀ��\�[�X�|�C���^�ύX�őΉ�����ƕ`�悪�����̂ŐV�K�����őΉ����Ă���
			air = nullptr;
			air = new Air(_dev.Get(), _fence.Get(), shadow->GetShadowMapResource(), shadowFactor->GetShadowFactorTextureResource());
			air->SetFrustum(camera->GetFrustum());
			air->SetParticipatingMedia(calculatedParticipatingMedia);

			shadowRenderingBlur->SetRenderingResourse(shadow->GetShadowRenderingResource());

			resourceManager[0]->SetAirResourceAndCreateView(air->GetAirTextureResource());
			resourceManager[1]->SetAirResourceAndCreateView(air->GetAirTextureResource());
		}
		//_gmemory->Commit(_cmdQueue.Get());
	}
}

void D3DX12Wrapper::CleanMemory()
{
	UnregisterClass(prepareRenderingWindow->GetWNDCCLASSEX().lpszClassName, prepareRenderingWindow->GetWNDCCLASSEX().hInstance);

	delete gPLSetting;
	gPLSetting = nullptr;

	prepareRenderingWindow = nullptr;

	delete peraGPLSetting;
	peraGPLSetting = nullptr;

	delete peraPolygon;
	peraPolygon = nullptr;

	delete settingImgui;
	settingImgui = nullptr;

	delete setRootSignature;
	setRootSignature = nullptr;

	delete peraSetRootSignature;
	peraSetRootSignature = nullptr;

	delete collisionManager;
	collisionManager = nullptr;

	delete oBBManager;
	oBBManager = nullptr;

	camera->CleanMemory();
	camera = nullptr;

	input = nullptr;
	fBXRootsignature = nullptr;
	fBXPipeline = nullptr;
	viewPort = nullptr;
	rect = nullptr;
	bBRootsignature = nullptr;
	bBPipeline = nullptr;

	delete sky;
	sky = nullptr;

	delete skyLUT;
	skyLUT = nullptr;

	delete shadowFactor;
	shadowFactor = nullptr;

	delete sun;
	sun = nullptr;

	delete shadow;
	shadow = nullptr;

	delete air;
	air = nullptr;

	delete shadowRenderingBlur;
	shadowRenderingBlur = nullptr;

	delete colorIntegraredBlur;
	colorIntegraredBlur = nullptr;

	delete ssaoBlur;
	ssaoBlur = nullptr;

	delete comBlur;
	comBlur = nullptr;

	delete integration;
	integration = nullptr;

	delete depthMapIntegration;
	depthMapIntegration = nullptr;

	delete calculateSSAO;
	calculateSSAO = nullptr;

	delete preFxaa;
	preFxaa = nullptr;

	for (auto& rm : resourceManager)
	{
		delete rm;
		rm = nullptr;
	}

	resourceManager.clear();
	vbViews.clear();
	ibViews.clear();

	// �ȉ��̂悤��ComPtr�ނ̓f�X�g���N�^�Ń�������������̂ŏ����s�v
	//_dev->Release();
	
	handle.ptr = 0;

	dHandles.clear();
	srvHeapAddresses.clear();
	rtvHeapPointer.ptr = 0;
	gHandle.ptr = 0;

	//indiceContainer.clear();
	//itIndiceFirsts.clear();
	//phongInfos.clear();
	//itPhonsInfos.clear();
	//materialAndTexturenameInfo.clear();
	//itMaterialAndTextureNames.clear();

}

void D3DX12Wrapper::AllKeyBoolFalse()
{
	inputW = false;
	inputLeft = false;
	inputRight = false;

	inputUp = false;
}

// Microsoft��directx sample���
// Initialize threads and events.
void D3DX12Wrapper::LoadContexts()
{
	struct threadwrapper
	{
		static unsigned int WINAPI thunk(LPVOID lpParameter) // LPVOID : �^�w��̂Ȃ��A32�r�b�g�|�C���^
		{
			ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
			instance->threadWorkTest(parameter->threadIndex);

			return 0;
		}
	};

	for (int i = 0; i < threadNum; i++)
	{
		// �ȉ��O��CreateEventW�ɂ��C�x���g�쐬 https://learn.microsoft.com/ja-jp/windows/win32/api/synchapi/nf-synchapi-createeventw
		m_workerBeginRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishedRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerSyncronize[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_threadParameters[i].threadIndex = i;

		// �X���b�h�쐬
		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr, // //SECURITY_ATTRIBUTES �\���̂ւ̃|�C���^ �܂���NULL
			0, //�X�^�b�N�T�C�Y�B0�ł��悢
			threadwrapper::thunk, // �X���b�h�֐��̃A�h���X
			reinterpret_cast<LPVOID>(&m_threadParameters[i]), // //�X���b�h�֐��ɓn�������A�܂���NULL ���̃P�[�X�ł͎��O�ɑ�������X���b�h�ԍ����i�[����m_threadHandles[i]�̃A�h���X�BLPVOID lpParameter�Ƃ��Ĉ�����B
			0, // //0(�������s) �܂���CREATE_SUSPENDED(�ꎞ��~)
			nullptr)); // //�X���b�h���ʎq�BNULL�ł��B

		assert(m_workerBeginRenderFrame[i] != NULL);
		//assert(m_workerFinishedRenderFrame[i] != NULL);
		//assert(m_threadHandles[i] != NULL);

	}
}

void D3DX12Wrapper::threadWorkTest(int num/*, ComPtr<ID3D12GraphicsCommandList> cmList*/)
{
	while (num >= 0 && num < threadNum)
	{
		WaitForSingleObject(m_workerBeginRenderFrame[num], INFINITE);
		//int fbxIndex = num;
		auto localCmdList = m_batchSubmit[num];

		// �}���`�^�[�Q�b�g���\�[�X�o���A����
		//color
		D3D12_RESOURCE_BARRIER barrierDescFBX = {};
		barrierDescFBX.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDescFBX.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDescFBX.Transition.Subresource = 0;
		barrierDescFBX.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierDescFBX.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		// normal
		D3D12_RESOURCE_BARRIER barrierDescNorm = {};
		barrierDescNorm.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDescNorm.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDescNorm.Transition.Subresource = 0;
		barrierDescNorm.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierDescNorm.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		// �X���b�h���ɎQ�Ɛ�𕪂���
		if (num == 0)
		{
			barrierDescFBX.Transition.pResource = resourceManager[0]->GetRenderingBuff().Get();
			barrierDescNorm.Transition.pResource = resourceManager[0]->GetNormalRenderingBuff().Get();
		}
		else if (num == 1)
		{
			barrierDescFBX.Transition.pResource = resourceManager[0]->GetRenderingBuff2().Get();
			barrierDescNorm.Transition.pResource = resourceManager[0]->GetNormalRenderingBuff2().Get();
		}

		//���\�[�X�o���A�F���\�[�X�ւ̕����̃A�N�Z�X�𓯊�����K�v�����邱�Ƃ��h���C�o�[�ɒʒm
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		localCmdList->ResourceBarrier(1, &barrierDescNorm);
		
		// ���f���`��
		localCmdList->RSSetViewports(1, viewPort);
		localCmdList->RSSetScissorRects(1, rect);

		// resourceManager[0]��rtv,dsv�ɏW�񂵂Ă���B��@�Ƃ��Ă̓C�}�C�`��...
		auto dsvhFBX = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
		dsvhFBX.ptr += num * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		auto handleFBX = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handleFBX.ptr += num * inc;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handles[2];
		uint32_t offset = 0; // start from No.2 RTV
		for (auto& handle : handles)
		{
			handle.InitOffsetted(handleFBX, inc * offset);
			offset += 2;
		}
		localCmdList->OMSetRenderTargets(2, handles, false, &dsvhFBX);
		localCmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

		//��ʃN���A
		float clearColor[4];
		clearColor[0] = 1.0f;
		clearColor[1] = 1.0f;
		clearColor[2] = 1.0f;
		clearColor[3] = 1.0f;

		/*localCmdList->ClearRenderTargetView(handleFBX, clearColor, 0, nullptr);*/
		localCmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
		localCmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
		XMFLOAT3 charaPos = {0,0,0};
		int lastSRVSetNum = 0;

		resourceManager[num]->GetMappedMatrix()->sponzaDraw = settingImgui->GetSponzaBoxChanged();
		resourceManager[num]->GetMappedMatrix()->airDraw = settingImgui->GetAirBoxChanged();
		resourceManager[num]->GetMappedMatrix()->brdfSpecularDraw = settingImgui->GetBRDFBoxChanged();

		for (int fbxIndex = 0; fbxIndex < modelPath.size(); ++fbxIndex)
		{
			localCmdList->SetGraphicsRootSignature(fBXRootsignature);
			localCmdList->SetPipelineState(fBXPipeline);

			// �A�t�B���ϊ��̑���͒P��X���b�h�̂ݎ��s����B�����X���b�h�ŏ�������ƁA���̕��������x���{������E���z���Ԃ��Ȃǈ��e�����o�邽�߁B
			if (num == 0)
			{
				// �L�[���͏����B�����蔻�菈�����܂߂�B
				if (input->CheckKey(DIK_W)) inputW = true;
				if (input->CheckKey(DIK_LEFT)) inputLeft = true;
				if (input->CheckKey(DIK_RIGHT)) inputRight = true;
				if (input->CheckKey(DIK_UP)) inputUp = true;

				if (resourceManager[fbxIndex]->GetIsAnimationModel()) // 20240315���_�̏����ł�resourceManager[1]�Ɍ��肳���B
				{
					// start character with idle animation
					resourceManager[fbxIndex]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);					
					charaPos.x = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
					charaPos.y = 1.5;
					charaPos.z = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];

					// W Key
					if (inputW)
					{
						resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
						XMVECTOR worldVec;
						worldVec.m128_f32[0] = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
						worldVec.m128_f32[1] = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1];
						worldVec.m128_f32[2] = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];
						worldVec.m128_f32[3] = 1;

						// ���s�ړ������ɃL�����N�^�[�̌��������]��������Z���ĕ����ς��B����ɂ���]�ړ������͕s�v�Ȃ̂ŁA1��0�ɂ���BY����]�̂ݑΉ����Ă���B
						auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, forwardSpeed), connanDirection);
						moveMatrix.r[0].m128_f32[0] = 1;
						moveMatrix.r[0].m128_f32[2] = 0;
						moveMatrix.r[2].m128_f32[0] = 0;
						moveMatrix.r[2].m128_f32[2] = 1;
						worldVec = XMVector4Transform(worldVec, moveMatrix); // ��������
						charaPos = collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, fbxIndex, worldVec, charaPos);
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->invProj = XMMatrixInverse(nullptr, XMMatrixInverse(nullptr, resourceManager[fbxIndex]->GetMappedMatrix()->proj));
						shadow->SetMoveMatrix(resourceManager[fbxIndex]->GetMappedMatrix()->world);
						calculateSSAO->SetViewRotMatrix(resourceManager[fbxIndex]->GetMappedMatrix()->view, XMMatrixInverse(nullptr, connanDirection));
					}

					// Left Key
					if (inputLeft)
					{
						connanDirection *= leftSpinMatrix;
						resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
						resourceManager[fbxIndex]->GetMappedMatrix()->rotation = connanDirection;
						shadow->SetRotationMatrix(connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->invProj = XMMatrixInverse(nullptr, XMMatrixInverse(nullptr, resourceManager[fbxIndex]->GetMappedMatrix()->proj));
						collisionManager->SetRotation(connanDirection);
						sun->ChangeSceneMatrix(rightSpinMatrix);
						sky->ChangeSceneMatrix(rightSpinMatrix);
						calculateSSAO->SetViewRotMatrix(resourceManager[fbxIndex]->GetMappedMatrix()->view, XMMatrixInverse(nullptr, connanDirection));
					}

					// Right Key
					if (inputRight)
					{
						connanDirection *= rightSpinMatrix;
						resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
						resourceManager[fbxIndex]->GetMappedMatrix()->rotation = connanDirection;
						shadow->SetRotationMatrix(connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->invProj = XMMatrixInverse(nullptr, XMMatrixInverse(nullptr, resourceManager[fbxIndex]->GetMappedMatrix()->proj));
						collisionManager->SetRotation(connanDirection);
						sun->ChangeSceneMatrix(leftSpinMatrix);
						sky->ChangeSceneMatrix(leftSpinMatrix);
						calculateSSAO->SetViewRotMatrix(resourceManager[fbxIndex]->GetMappedMatrix()->view, XMMatrixInverse(nullptr, connanDirection));
					}
				}

				// W Key
				if (inputW && !resourceManager[fbxIndex]->GetIsAnimationModel())
				{
					resourceManager[fbxIndex]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;
				}
			}

			// ���L�����N�^�[�̉�]�s���]�u�������̂�GPU�֓n���BDirectX�ł�CPU����󂯎�����s��͓]�u����GPU�֓]�������B(or GPU�œ]�u�����? �ڂ����͕s��) CPU���ŗ��p���Ă����]�s���GPU�ŗ��p�����]�s������킹�邱�Ƃ�GPU���ł̖@����]�v�Z����肭�����Ă���F������...
			resourceManager[fbxIndex]->GetMappedMatrix()->charaRot = XMMatrixTranspose(connanDirection);

			//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
			localCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/);

			//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
			localCmdList->IASetVertexBuffers(0, 1, vbViews[fbxIndex]);

			//���C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
			localCmdList->IASetIndexBuffer(ibViews[fbxIndex]);

			//�f�B�X�N���v�^�q�[�v�ݒ肨��уf�B�X�N���v�^�q�[�v�ƃ��[�g�p�����[�^�̊֘A�t��	
			localCmdList->SetDescriptorHeaps(1, srvHeapAddresses[fbxIndex].GetAddressOf());

			auto dHandle = dHandles[fbxIndex];
			localCmdList->SetGraphicsRootDescriptorTable(0, dHandle); // WVP Matrix(Numdescriptor : 1)
			dHandle.ptr += cbv_srv_Size * 8; // air�A�h���X�܂Ŕ��

			int textureindex = 2;
			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // air (ptr num9)
			++textureindex;
			dHandle.ptr += cbv_srv_Size;

			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // vsm (ptr num10)
			++textureindex;
			dHandle.ptr += cbv_srv_Size;

			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // depthmap (ptr num11)
			++textureindex;
			dHandle.ptr += cbv_srv_Size * 3; // �V���h�E�}�b�v����і@���摜3���̗̈�͎g��Ȃ��̂Ŕ�΂��B

			auto itIndiceFirst = itIndiceFirsts[fbxIndex];
			auto ofst = 0;
			ofst += itIndiceFirst->second.indices.size() * num;
			itIndiceFirst += num;
			int indiceContainerSize = indiceContainer[fbxIndex].size();

			auto itPhonsInfo = itPhonsInfos[fbxIndex];
			itPhonsInfo += num;

			//auto mappedPhong = resourceManager[fbxIndex]->GetMappedPhong();

			auto itMaterialAndTextureName = itMaterialAndTextureNames[fbxIndex];
			itMaterialAndTextureName += textureIndexes[fbxIndex][0] * num;
			//itMATCnt = /*0*/num;
			auto matTexSize = matTexSizes[fbxIndex];
			auto tHandle = dHandle;
			tHandle.ptr += cbv_srv_Size * indiceContainerSize + cbv_srv_Size * textureIndexes[fbxIndex][0] * num; // �����X���b�h1�̂ݍX�ɃX���b�h0����������ŏ��̃e�N�X�`�������@+ cbv_srv_Size * textureIndexes[0].second
			int texStartIndex = textureindex; // �e�N�X�`�����i�[����f�B�X�N���v�^�e�[�u���ԍ��̊J�n�ʒu
			auto textureTableStartIndex = texStartIndex; // 3 is number of texture memory position in SRV
			auto indiceSize = 0;
			int itMATCnt = 0;
			itMATCnt += textureIndexes[fbxIndex][0] * num;
			for (int i = num; i < indiceContainerSize; i += threadNum)
			{
				//localCmdList->SetGraphicsRootDescriptorTable(1, dHandle); // Phong Material Parameters(Numdescriptor : 3)
				//mappedPhong[i]->diffuse[0] = itPhonsInfo->second.diffuse[0];
				//mappedPhong[i]->diffuse[1] = itPhonsInfo->second.diffuse[1];
				//mappedPhong[i]->diffuse[2] = itPhonsInfo->second.diffuse[2];
				//mappedPhong[i]->ambient[0] = itPhonsInfo->second.ambient[0];
				//mappedPhong[i]->ambient[1] = itPhonsInfo->second.ambient[1];
				//mappedPhong[i]->ambient[2] = itPhonsInfo->second.ambient[2];
				//mappedPhong[i]->emissive[0] = itPhonsInfo->second.emissive[0];
				//mappedPhong[i]->emissive[1] = itPhonsInfo->second.emissive[1];
				//mappedPhong[i]->emissive[2] = itPhonsInfo->second.emissive[2];
				//mappedPhong[i]->bump[0] = itPhonsInfo->second.bump[0];
				//mappedPhong[i]->bump[1] = itPhonsInfo->second.bump[1];
				//mappedPhong[i]->bump[2] = itPhonsInfo->second.bump[2];
				//mappedPhong[i]->specular[0] = itPhonsInfo->second.specular[0];
				//mappedPhong[i]->specular[1] = itPhonsInfo->second.specular[1];
				//mappedPhong[i]->specular[2] = itPhonsInfo->second.specular[2];
				//mappedPhong[i]->reflection[0] = itPhonsInfo->second.reflection[0];
				//mappedPhong[i]->reflection[1] = itPhonsInfo->second.reflection[1];
				//mappedPhong[i]->reflection[2] = itPhonsInfo->second.reflection[2];
				//mappedPhong[i]->transparency = itPhonsInfo->second.transparency;

				if (matTexSize > 0) {
					//std::string currentMeshName = itMaterialAndTextureName->first;
					// ���p�X�͊��ɓ]�����Ɏg�p�ρB�}�e���A������char��1byte�ɏ��������Ĕ�r����΂����̂ł́H
					while (itMaterialAndTextureName->first == itPhonsInfo->first) // TODO:second���g���Ă��Ȃ����e�ʂ̖��ʌ����Ȃ̂ŏ���
					{
						localCmdList->SetGraphicsRootDescriptorTable(textureTableStartIndex, tHandle); // index of texture
						tHandle.ptr += cbv_srv_Size;
						++textureTableStartIndex;
						++itMATCnt;

						if (itMATCnt == matTexSize)
						{
							break;
						}
						++itMaterialAndTextureName/*itMaterialAndTextureName += 2*/;

						if (itMaterialAndTextureName == materialAndTexturenameInfo[fbxIndex].end())
						{
							break;
						}
					}
				}

				indiceSize = itIndiceFirst->second.indices.size(); // ���T�C�Y�݂̂�array��p�ӂ��Ă݂�
				localCmdList->DrawIndexedInstanced(indiceSize, 1, ofst, 0, 0);
				ofst += indiceSize;

				if (i + threadNum >= indiceContainerSize)
				{					
					break;
				}
				itIndiceFirst += threadNum;
				itPhonsInfo += threadNum;

				auto itNext = itIndiceFirst - 1;
				ofst += itNext->second.indices.size();
				tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [1]�X���b�h�ŏ������鎟�̊Y�����f���C���f�b�N�X�̃e�N�X�`����
				
				itMaterialAndTextureName += textureIndexes[fbxIndex][i + 1];
				itMATCnt += textureIndexes[fbxIndex][i + 1];
				textureTableStartIndex = texStartIndex; // init
			}

			// resourceManager[0]��sponza���f���̏���ێ����Ă���A[1]�̓L�����N�^�[���f���̏���ێ����Ă���B
			// ���[0]��view�s�񂪌v�Z�����̂ŁA[0]�ɂ����̌��ʂ𔽉f�����邱�Ƃ�sponza����]����ɍ��킹�ăL�����N�^�[�Ɠ��l��(�L�����N�^�[�����I�[�r�b�g)��]����悤�ɂȂ�B
			// num == 0 && fbxIndex == 1�ŏ������d�����Ȃ��悤�ɂ��Ă���B�Ȃ��d�����Ă����ɖ��͂Ȃ��B
			if (inputLeft)
			{
				resourceManager[0]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;

			}
			if (inputRight)
			{
				resourceManager[0]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;
			}
		}

		bool colliderDraw = settingImgui->GetCollisionBoxChanged();
		// �R���C�_�[�`�� thread1(num=0)�ŃL�����N�^�[�X�t�B�A�Athread2��OBB��`�悷��
		if (num == 0)
		{
			auto mappedMatrix = resourceManager[0]->GetMappedMatrix();
			collisionManager->SetMatrix(mappedMatrix->world, mappedMatrix->view, mappedMatrix->proj);
			collisionManager->SetCharaPos(charaPos);
			collisionManager->SetDraw(colliderDraw, localCmdList.Get());
		}
		else
		{
			auto mappedMatrix = resourceManager[0]->GetMappedMatrix();
			oBBManager->SetMatrix(mappedMatrix->world, mappedMatrix->view, mappedMatrix->proj);
			oBBManager->SetDraw(colliderDraw, localCmdList.Get());
		}

		// �}���`�^�[�Q�b�g���\�[�X�o���A����
		// color
		barrierDescFBX.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDescFBX.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		// normal
		barrierDescNorm.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDescNorm.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		localCmdList->ResourceBarrier(1, &barrierDescNorm);
		localCmdList->Close();

		SetEvent(m_workerFinishedRenderFrame[num]); // end drawing.
	}
}

//void D3DX12Wrapper::DrawShrinkTextureForBlur(unsigned int modelNum, UINT buffSize)
//{
//	_cmdList->SetPipelineState(bloomGPLSetting->GetPipelineState().Get());
//	_cmdList->SetGraphicsRootSignature(bloomRootSignature->GetRootSignature().Get());
//
//	// set vertex buffer
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	_cmdList->IASetVertexBuffers(0, 1, peraPolygon->GetVBView());
//
//	// No need to set luminance buffer to shader resource, cause other method implement it.
//
//	// high luminance blur renderer status to
//	D3D12_RESOURCE_BARRIER barrierDesc4Shrink = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[1].Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Shrink);
//
//	// model blur renderer status to
//	D3D12_RESOURCE_BARRIER barrierDesc4ShrinkModel = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[2].Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4ShrinkModel);
//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE handles[2];
//	auto baseH = bufferHeapCreator[modelNum]->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
//	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	uint32_t offset = 4;
//	for (auto& handle : handles)
//	{
//		handle.InitOffsetted(baseH, incSize * offset);
//		offset += 1;
//	}	
//
//	_cmdList->OMSetRenderTargets(2, handles, false, nullptr);
//	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	_cmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
//	_cmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
//
//	_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[modelNum]->GetMultipassSRVHeap().GetAddressOf());
//
//	// bloom texture
//	auto srvHandle = bufferHeapCreator[modelNum]->GetMultipassSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//	srvHandle.ptr += buffSize; // model texture
//	_cmdList->SetGraphicsRootDescriptorTable(1, srvHandle);
//	srvHandle.ptr += buffSize; // gaussian value
//	_cmdList->SetGraphicsRootDescriptorTable(2, srvHandle); // table[2] is for gaussian value
//	srvHandle.ptr += buffSize * 6; // bloom texture
//	_cmdList->SetGraphicsRootDescriptorTable(0, srvHandle);
//
//	auto desc = bufferHeapCreator[modelNum]->GetBloomBuff()[0]->GetDesc();
//	D3D12_VIEWPORT vp = {};
//	D3D12_RECT sr = {};
//
//	vp.MaxDepth = 1.0f;
//	vp.MinDepth = 0.0f;
//	vp.Height = desc.Height / 2;
//	vp.Width = desc.Width / 2;
//	sr.top = 0;
//	sr.left = 0;
//	sr.right = vp.Width;
//	sr.bottom = vp.Height;
//
//	for (int i = 0; i < 8; i++)
//	{
//		_cmdList->RSSetViewports(1, &vp);
//		_cmdList->RSSetScissorRects(1, &sr);
//		_cmdList->DrawInstanced(4, 1, 0, 0);
//
//		// draw and shift down to draw next texture
//		sr.top += vp.Height;
//		vp.TopLeftX = 0;
//		vp.TopLeftY = sr.top;
//
//		// halve width and height
//		vp.Width /= 2;
//		vp.Height /= 2;
//		sr.bottom = sr.top + vp.Height;
//	}
//
//	// change resource from render target to shader resource
//	barrierDesc4Shrink.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	barrierDesc4Shrink.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	_cmdList->ResourceBarrier(1, &barrierDesc4Shrink);
//
//	// change resource from render target to shader resource
//	barrierDesc4ShrinkModel.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	barrierDesc4ShrinkModel.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	_cmdList->ResourceBarrier(1, &barrierDesc4ShrinkModel);
//}
//


void D3DX12Wrapper::DrawBackBuffer(UINT buffSize)
{

	bbIdx = _swapChain->GetCurrentBackBufferIndex();//���݂̃o�b�N�o�b�t�@���C���f�b�N�X�ɂĎ擾

	bool isWindowSizeChanged = prepareRenderingWindow->GetWindowSizeChanged();
	// �E�B���h�E�T�C�Y�ύX���������ꍇ�̏���
	if (isWindowSizeChanged)
	{
		// �X���b�v�`�F�[���̃��T�C�Y�̂��߁A�Q�Ƃ��Ă��郊�\�[�X(_backBuffers[])��������Ă��K�v������B
		// ���̂��߂�GPU�̌��t���[����������������܂�(�t���b�V��)�҂B
		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);
		while (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//�C�x���g�����҂�
			WaitForSingleObject(event, INFINITE);
			//�C�x���g�n���h�������
			CloseHandle(event);
		}

		UINT clientWidth = prepareRenderingWindow->GetWindowWidth();
		UINT clientHeight = prepareRenderingWindow->GetWindowHeight();

		// �_�u���o�b�t�@�݌v�̂��߁A0,1,2�Ƃ������}�W�b�N�i���o�[���g���Ă���...
		// �o�b�N�o�b�t�@�p2�̃��\�[�X�����ł����s�ɉe���͂Ȃ��B���ArtvHeap�����Z�b�g�Acmdallocator/list4�����蓖�Ăă��Z�b�g���Ă�warning���o��B
		// WinPixGpuCapturer.dll warning: Warning: shared resources HANDLES have been recycled. This may cause incorrect captures.
		_backBuffers[0].Reset();
		_backBuffers[1].Reset();

		//rtvHeap->Release();
		//rtvHeap.Reset();
		//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		//rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		//rtvHeapDesc.NumDescriptors = 2;
		//rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//rtvHeapDesc.NodeMask = 0;
		//result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf()));

		DXGI_SWAP_CHAIN_DESC desc = {};
		_swapChain->GetDesc(&desc);
		_swapChain->ResizeBuffers(2, clientWidth, clientHeight, desc.BufferDesc.Format, desc.Flags);

		// ���̏������Ȃ��ƈȉ��G���[����������
		// A command list, which writes to a swapchain back buffer, may only be executed when that back buffer is the back buffer that will be presented during the next call to Present* .Such a back buffer is also referred to as the �gcurrent back buffer�h.
		bbIdx = _swapChain->GetCurrentBackBufferIndex();
		
		auto _handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (int idx = 0; idx < 2; idx++)
		{  
			result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			_dev->CreateRenderTargetView
			(
				_backBuffers[idx].Get(),
				nullptr,
				_handle
			);

			_handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// WIndow�Ǘ��N���X�֏���������ʒm����
		prepareRenderingWindow->SetChangeFinished();
	}

	_cmdList3->RSSetViewports(1, changeableViewport);
	_cmdList3->RSSetScissorRects(1, changeableRect);

	// �ޯ��ޯ̧�ɕ`�悷��
	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂɕύX����
	barrierDesc4BackBuffer = CD3DX12_RESOURCE_BARRIER::Transition
	(
		_backBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);

	rtvHeapPointer = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapPointer.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList3->OMSetRenderTargets(1, &rtvHeapPointer, false, /*&dsvh*/nullptr);

	_cmdList3->ClearRenderTargetView(rtvHeapPointer, clearColor, 0, nullptr);

	// �쐬����ø����̗��p����
	_cmdList3->SetGraphicsRootSignature(bBRootsignature);

	_cmdList3->SetDescriptorHeaps(1, preFxaa->GetDescriptorHeap().GetAddressOf());

	gHandle = preFxaa->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle);
	gHandle.ptr += inc;
	_cmdList3->SetGraphicsRootDescriptorTable(13, gHandle);

	_cmdList3->SetPipelineState(bBPipeline);

	_cmdList3->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_cmdList3->IASetVertexBuffers(0, 1, peraPolygon->GetVBView());

	_cmdList3->DrawInstanced(4, 1, 0, 0);

	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂ��猳�ɖ߂�
	barrierDesc4BackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc4BackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);
}

//void D3DX12Wrapper::DrawEffect()
//{
//	_efkHandle = _efkManager->Play(_effect, 0, 0, 0);
//	_efkManager->Update();
//	for (int j = 0; j < 4; ++j)
//	{
//		for (int k = 0; k < 4; ++k)
//		{
//			fkViewMat.Values[j][k] = pmdMaterialInfo[0]->mapMatrix->view.r[j].m128_f32[k];
//			fkProjMat.Values[j][k] = pmdMaterialInfo[0]->mapMatrix->proj.r[j].m128_f32[k];
//		}
//	}
//	_efkRenderer->SetCameraMatrix(fkViewMat);
//	_efkRenderer->SetProjectionMatrix(fkProjMat);
//
//	_efkMemoryPool->NewFrame();
//	EffekseerRendererDX12::BeginCommandList(_efkCmdList, _cmdList.Get());
//	_efkRenderer->BeginRendering();
//	_efkManager->Draw();
//	_efkRenderer->EndRendering();
//	EffekseerRendererDX12::EndCommandList(_efkCmdList);
//}
//
//void D3DX12Wrapper::DirectXTKInit()
//{
//	// Initialize GraphicsMemory object
//	_gmemory = new GraphicsMemory(_dev.Get());
//
//	// Initialize SpriteBatch object
//	ResourceUploadBatch resUploadBatch(_dev.Get());
//	resUploadBatch.Begin();
//	RenderTargetState rsState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
//	SpriteBatchPipelineStateDescription pd(rsState);
//	_spriteBatch = new SpriteBatch(_dev.Get(), resUploadBatch, pd);
//	_spriteBatch->SetViewport(prepareRenderingWindow->GetViewPort());
//
//	bufferHeapCreator[0]->CreateSpriteFontHeap(_dev);
//
//	// Initialize SpriteFont object
//	auto _heapSpriteFont = bufferHeapCreator[0]->GetSpriteFontHeap();
//	_spriteFont = new SpriteFont
//	(
//		_dev.Get(),
//		resUploadBatch,
//		L"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\font\\kanji.spritefont",
//		_heapSpriteFont->GetCPUDescriptorHandleForHeapStart(),
//		_heapSpriteFont->GetGPUDescriptorHandleForHeapStart()
//	);
//
//	auto _future = resUploadBatch.End(_cmdQueue.Get());
//
//	_future.wait();
//}
//
//void D3DX12Wrapper::DrawSpriteFont()
//{
//	_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[0]->GetSpriteFontHeap().GetAddressOf());
//	_spriteBatch->Begin(_cmdList.Get());
//	_spriteFont->DrawString
//	(
//		_spriteBatch,
//		L"DirectX12�̖�����",
//		XMFLOAT2(102,102),
//		Colors::Black
//	);
//	_spriteFont->DrawString
//	(
//		_spriteBatch,
//		L"DirectX12�̖�����",
//		XMFLOAT2(100, 100),
//		Colors::Yellow
//	);
//	_spriteBatch->End();
//}

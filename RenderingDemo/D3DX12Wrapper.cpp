#include <stdafx.h>
#include <D3DX12Wrapper.h>

#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// effekseer
#pragma comment(lib, "EffekseerRendererDx12.lib")
#pragma comment(lib, "Effekseer.lib")
#pragma comment(lib, "LLGI.lib")
#pragma comment(lib, "EffekseerRendererCommon.lib")
#pragma comment(lib, "EffekseerRendererLLGI.lib")


using namespace DirectX;
using namespace Microsoft::WRL;

using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;
D3DX12Wrapper* D3DX12Wrapper::instance = nullptr;

D3DX12Wrapper::D3DX12Wrapper()
{
	instance = this;
};

// �㏈��
void D3DX12Wrapper::Terminate()
{

};

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

	_fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence./*ReleaseAnd*/GetAddressOf()));
}

//#ifdef _DEBUG
bool D3DX12Wrapper::PrepareRendering() {
//#else
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
//{
//#endif

	strModelPath =
	{
		"C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBXConnan_Walking.fbx"
	};

	strModelNum = strModelPath.size();

	// SetRootSignatureBase�N���X�̃C���X�^���X��
	setRootSignature = new SetRootSignature;	

	// SettingShaderCompile�N���X�̃C���X�^���X��
	settingShaderCompile = new SettingShaderCompile;

	// VertexInputLayout�N���X�̃C���X�^���X��
	vertexInputLayout = new VertexInputLayout;

	// GraphicsPipelineSetting�N���X�̃C���X�^���X��
	gPLSetting = new GraphicsPipelineSetting(vertexInputLayout);
	delete vertexInputLayout;

	// �����_�����O�E�B���h�E�ݒ�
	prepareRenderingWindow = new PrepareRenderingWindow;
	prepareRenderingWindow->CreateAppWindow();

	// TextureLoader�N���X�̃C���X�^���X��
	textureLoader = new TextureLoader;

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
	//bufferGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/);
	//bufferSetRootSignature = new PeraSetRootSignature;
	//bufferShaderCompile = new BufferShaderCompile;

	//// ���C�g�}�b�v�֘A
	//lightMapGPLSetting = new LightMapGraphicsPipelineSetting(vertexInputLayout);
	//lightMapRootSignature = new SetRootSignature;
	//lightMapShaderCompile = new LightMapShaderCompile;

	//// bloom	
	//bloomGPLSetting = new PeraGraphicsPipelineSetting(peraLayout);
	//bloomRootSignature = new PeraSetRootSignature;
	//bloomShaderCompile = new BloomShaderCompile;

	////AO
	//aoGPLSetting = new AOGraphicsPipelineSetting(vertexInputLayout);
	//aoRootSignature = new SetRootSignature;
	//aoShaderCompile = new AOShaderCompile;

	// Collision
	collisionRootSignature = new CollisionRootSignature;
	colliderGraphicsPipelineSetting = new ColliderGraphicsPipelineSetting(peraLayout);
	collisionShaderCompile = new SettingShaderCompile;
	delete peraLayout;

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
	swapChainDesc.Width = prepareRenderingWindow->GetWindowWidth();
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

void D3DX12Wrapper::EffekseerInit()
{
	_efkManager = Effekseer::Manager::Create(8000);
	// DirectX�͍���n�̂��߁A����ɍ��킹��
	_efkManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

	auto graphicsDevice = EffekseerRendererDX12::CreateGraphicsDevice(_dev.Get(), _cmdQueue.Get(), 2);
	
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
	_efkRenderer = EffekseerRendererDX12::Create(graphicsDevice, &format, 1, DXGI_FORMAT_UNKNOWN, false, 8000);
	_efkMemoryPool = EffekseerRenderer::CreateSingleFrameMemoryPool(_efkRenderer->GetGraphicsDevice());
	_efkCmdList = EffekseerRenderer::CreateCommandList(_efkRenderer->GetGraphicsDevice(), _efkMemoryPool);

	_efkRenderer->SetCommandList(_efkCmdList);

	// �`�惂�W���[���̐ݒ�
	_efkManager->SetSpriteRenderer(_efkRenderer->CreateSpriteRenderer());
	_efkManager->SetRibbonRenderer(_efkRenderer->CreateRibbonRenderer());
	_efkManager->SetRingRenderer(_efkRenderer->CreateRingRenderer());
	_efkManager->SetTrackRenderer(_efkRenderer->CreateTrackRenderer());
	_efkManager->SetModelRenderer(_efkRenderer->CreateModelRenderer());

	// �e�N�X�`���A���f���A�J�[�u�A�}�e���A�����[�_�[�̐ݒ肷��B
	// ���[�U�[���Ǝ��Ŋg���ł���B���݂̓t�@�C������ǂݍ���ł���B
	_efkManager->SetTextureLoader(_efkRenderer->CreateTextureLoader());
	_efkManager->SetModelLoader(_efkRenderer->CreateModelLoader());
	_efkManager->SetMaterialLoader(_efkRenderer->CreateMaterialLoader());
	_efkManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	// �G�t�F�N�g���̂̐ݒ�
	_effect = Effekseer::Effect::Create
	(
		_efkManager,
		(const EFK_CHAR*)L"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\EffekseerTexture\\10\\SimpleLaser.efk",
		1.0f,
		(const EFK_CHAR*)L"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\EffekseerTexture\\10"
	);
	


	/*_efkHandle = _efkManager->Play(_effect, 0, 0, 0);*/

}

bool D3DX12Wrapper::ResourceInit() {
	//�����\�[�X������
	

	// 0 texture model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\BattleField.txt");

	// 3 texture model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\Ziggrat.txt");

	modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\Sponza.bin");

	// 4 textures model
	/*modelPath.push_back("C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\Connan.txt");*/
	modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\Connan.bin");
	
	resourceManager.resize(modelPath.size());
	FBXInfoManager fbxInfoManager;

	camera = Camera::GetInstance();
	camera->Init(prepareRenderingWindow);

	for (int i = 0; i < modelPath.size(); ++i)
	{
		// FBXInfoManager Instance
		fbxInfoManager = FBXInfoManager::Instance();
		fbxInfoManager.Init(modelPath[i]);
		
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

	// �ײ�ް�p
	if (FAILED(collisionRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}

	

	//// �\���p
	//if (FAILED(bufferSetRootSignature->SetRootsignatureParam(_dev)))
	//{
	//	return false;
	//}

	//// ���C�g�}�b�v�p
	//if (FAILED(lightMapRootSignature->SetRootsignatureParam(_dev)))
	//{
	//	return false;
	//}

	//// bloom
	//if (FAILED(bloomRootSignature->SetRootsignatureParam(_dev)))
	//{
	//	return false;
	//}

	//// AO
	//if (FAILED(aoRootSignature->SetRootsignatureParam(_dev)))
	//{
	//	return false;
	//}
	
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

	// �o�b�N�o�b�t�@�`��p
	std::string bufferVs = "PeraVertex.hlsl";
	std::string bufferPs = "PeraPixel.hlsl";
	auto bufferPathPair = Utility::GetHlslFilepath(bufferVs, bufferPs);
	auto mBlobs = peraShaderCompile->PeraSetShaderCompile(peraSetRootSignature, _vsMBlob, _psMBlob,
		bufferPathPair.first, "vs",
		bufferPathPair.second, "ps");
	if (mBlobs.first == nullptr or mBlobs.second == nullptr) return false;
	_vsMBlob = mBlobs.first;
	_psMBlob = mBlobs.second;
	delete peraShaderCompile;

	// �R���C�_�[�`��p
	std::string collisionVs = "CollisionVertex.hlsl";
	std::string collisionPs = "CollisionPixel.hlsl";
	auto collisionPathPair = Utility::GetHlslFilepath(collisionVs, collisionPs);
	auto colliderBlobs = collisionShaderCompile->CollisionSetShaderCompile(collisionRootSignature, _vsCollisionBlob, _psCollisionBlob,
		collisionPathPair.first, "vs",
		collisionPathPair.second, "ps");
	if (colliderBlobs.first == nullptr or colliderBlobs.second == nullptr) return false;
	_vsCollisionBlob = colliderBlobs.first;
	_psCollisionBlob = colliderBlobs.second;
	delete collisionShaderCompile;
	
	//// �\���p
	//auto bufferBlobs = bufferShaderCompile->SetPeraShaderCompile(bufferSetRootSignature, _vsBackbufferBlob, _psBackbufferBlob);
	//if (bufferBlobs.first == nullptr or bufferBlobs.second == nullptr) return false;
	//_vsBackbufferBlob = bufferBlobs.first;
	//_psBackbufferBlob = bufferBlobs.second;

	//// ���C�g�}�b�v�p
	//auto lightMapBlobs = lightMapShaderCompile->SetShaderCompile(lightMapRootSignature, _lightMapVSBlob, _lightMapPSBlob);
	//if (lightMapBlobs.first == nullptr) return false;
	//_lightMapVSBlob = lightMapBlobs.first;
	//_lightMapPSBlob = lightMapBlobs.second; // �������nullptr

	//// bloom
	//auto bloomBlobs = bloomShaderCompile->SetPeraShaderCompile(bloomRootSignature, _bloomVSBlob, _bloomPSBlob);
	//if (bloomBlobs.first == nullptr or bufferBlobs.second == nullptr) return false;
	//_bloomVSBlob = bloomBlobs.first; // �������nullpt
	//_bloomPSBlob = bloomBlobs.second;

	//// AO
	//auto aoBlobs = aoShaderCompile->SetShaderCompile(aoRootSignature, _aoVSBlob, _aoPSBlob);
	//if (aoBlobs.first == nullptr or aoBlobs.second == nullptr) return false;
	//_aoVSBlob = aoBlobs.first; // �������nullpt
	//_aoPSBlob = aoBlobs.second;

// ����������3�F���_���̓��C�A�E�g�̍쐬�y��
// ����������4�F�p�C�v���C����ԃI�u�W�F�N�g(PSO)��Desc�L�q���ăI�u�W�F�N�g�쐬
	result = gPLSetting->CreateGPStateWrapper(_dev, setRootSignature, _vsBlob, _psBlob);
	
	// �R���C�_�[�p
	result = colliderGraphicsPipelineSetting->CreateGPStateWrapper(_dev, collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);

	// �ޯ��ޯ̧�p
	result = peraGPLSetting->CreateGPStateWrapper(_dev, peraSetRootSignature, _vsMBlob, _psMBlob);

	//// �\���p
	//result = bufferGPLSetting->CreateGPStateWrapper(_dev, bufferSetRootSignature, _vsBackbufferBlob, _psBackbufferBlob);

	//// ���C�g�}�b�v�p
	//result = lightMapGPLSetting->CreateGPStateWrapper(_dev, lightMapRootSignature, _lightMapVSBlob, _lightMapPSBlob);

	//// for shrinked bloom creating
	//result = bloomGPLSetting->CreateGPStateWrapper(_dev, bloomRootSignature, _bloomVSBlob, _bloomPSBlob);

	//// AO
	//result = aoGPLSetting->CreateGPStateWrapper(_dev, aoRootSignature, _aoVSBlob, _aoPSBlob);

// ����������5�F�R�}���h���X�g����
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator2.Get(), nullptr, IID_PPV_ARGS(_cmdList2.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator3.Get(), nullptr, IID_PPV_ARGS(_cmdList3.ReleaseAndGetAddressOf()));
	m_batchSubmit[0] = _cmdList.Get();
	m_batchSubmit[1] = _cmdList2.Get();
// ����������6�F�R�}���h���X�g�̃N���[�Y(�R�}���h���X�g�̎��s�O�ɂ͕K���N���[�Y����)

// ����������7�F�e�o�b�t�@�[���쐬���Ē��_����ǂݍ���

	
	//�t�@�C���`�����̃e�N�X�`�����[�h����
	textureLoader->LoadTexture();

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
	delete textureLoader;
	delete textureTransporter;
	//// �K�E�V�A���ڂ����p�E�F�C�g�A�o�b�t�@�[�쐬�A�}�b�s���O�A�f�B�X�N���v�^�q�[�v�쐬�A�r���[�쐬�܂�
	//auto weights = Utility::GetGaussianWeight(8, 5.0f);
	//bufferHeapCreator[i]->CreateConstBufferOfGaussian(_dev, weights);
	//mappingExecuter[i]->MappingGaussianWeight(weights);
	////bufferHeapCreator->CreateEffectHeap(_dev);
	////viewCreator->CreateCBV4GaussianView(_dev);

	//// �}���`�p�X�p�r���[�쐬
	peraPolygon->CreatePeraView(_dev);
	//viewCreator[i]->CreateRTV4Multipasses(_dev);
	//viewCreator[i]->CreateSRV4Multipasses(_dev);

// ����������9�F�t�F���X�̐���
// ����������10�F�C�x���g�n���h���̍쐬
// ����������11�FGPU�̏��������҂�

// Imgui�Ǝ��̏����ݒ�
	settingImgui = new SettingImgui;

	if (FAILED(settingImgui->Init(_dev, prepareRenderingWindow)))
	{
		return false;
	}	

	//for (int i = 0; i < strModelNum; ++i)
	//{
	//	bufferHeapCreator[i]->CreateBuff4Imgui(_dev, settingImgui->GetPostSettingSize());
	//	viewCreator[i]->CreateCBV4ImguiPostSetting(_dev);
	//	mappingExecuter[i]->MappingPostSetting();
	//}

//// DirectXTK�Ǝ��̏����ݒ�
//	DirectXTKInit();
//	
	for (auto& reManager : resourceManager)
	{
		reManager->ClearReference();
	}

	viewPort = prepareRenderingWindow->GetViewPortPointer();
	rect = prepareRenderingWindow->GetRectPointer();

	// Sky�ݒ�
	calculatedParticipatingMedia = participatingMedia.calculateUnit();

	sun = new Sun(_dev.Get(), camera);
	sun->Init();
	
	shadowFactor = new ShadowFactor(_dev.Get(), _fence.Get());
	shadowFactor->SetParticipatingMedia(calculatedParticipatingMedia);
	auto shadowFactorResource = shadowFactor->GetShadowFactorTextureResource();

	skyLUT = new SkyLUT(_dev.Get()/*, _fence.Get()*/, shadowFactorResource.Get());
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

	//shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get());
	//skyLUT->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);

	auto skyLUTResource = skyLUT->GetSkyLUTRenderingResource();
	sky = new Sky(_dev.Get(), _fence.Get(), skyLUTResource.Get());
	sky->SetSceneInfo(camera->GetWorld());
	
	camera->CalculateFrustum();
	camera->SetDummyFrustum();
	sky->SetFrustum(camera->GetFrustum());

	sun->SetShadowFactorResource(shadowFactorResource.Get());

	shadow = new Shadow(_dev.Get());
	shadow->Init();
	
	air = new Air(_dev.Get(), _fence.Get(), shadow->GetShadowMapREsource(), shadowFactor->GetShadowFactorTextureResource());
	air->SetFrustum(camera->GetFrustum());
	air->SetParticipatingMedia(calculatedParticipatingMedia);
	
	// resourceManager[0]�݂̂Ɋi�[...
	resourceManager[0]->SetSunResourceAndCreateView(sun->GetRenderResource());
	resourceManager[0]->SetSkyResourceAndCreateView(sky->GetSkyLUTRenderingResource());
	resourceManager[0]->SetImGuiResourceAndCreateView(settingImgui->GetImguiRenderingResource());
	// air(�{�����[�����C�e�B���O)�̓I�u�W�F�N�g�ƃL�����N�^�[�ŗ��p
	resourceManager[0]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	resourceManager[1]->SetAirResourceAndCreateView(air->GetAirTextureResource());

	return true;
}

void D3DX12Wrapper::Run() {
	MSG msg = {};
	cbv_srv_Size = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//// �G�t�F�N�g�̍Đ�
	//_efkHandle = _efkManager->Play(_effect, 0, 0, 0);
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
			
			//resourceManager[i]->MotionUpdate(motionNames.at(""), motion->second[0].size());

			resourceManager[i]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);
		}
	}

	// �Փ˔��菀��
	collisionManager = new CollisionManager(_dev, resourceManager);
	//connanDirection = resourceManager[1]->GetMappedMatrix()->world;
	leftSpinMatrix = XMMatrixRotationY(-turnSpeed);
	XMVECTOR det;
	rightSpinMatrix = XMMatrixInverse(&det, leftSpinMatrix);//XMMatrixRotationY(turnSpeed);
	//box2 = collisionManager->GetBoundingSpherePointer();

	//��eigen test
	Matrix3d leftSpinEigen;
	Vector3d axis;
	axis << 0, 1, 0;  //y�����w��
	leftSpinEigen = AngleAxisd(/*M_PI*/PI*0.006f, axis);  //Z�������90�x�����v���ɉ�]
	leftSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	leftSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	leftSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	leftSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	leftSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	leftSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	leftSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	leftSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	leftSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);

	leftSpinEigen = AngleAxisd(-/*M_PI*/PI*0.006f, axis);  //Z�������90�x�����v���ɉ�]
	rightSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	rightSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	rightSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	rightSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	rightSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	rightSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	rightSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	rightSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	rightSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);

	angleUpMatrix = XMMatrixRotationX(turnSpeed);
	Matrix3d angleUp;
	Vector3d axisX;
	axisX << 1, 0, 0;
	angleUp = AngleAxisd(/*M_PI*/PI * 0.006f, axisX);
	angleUpMatrix.r[0].m128_f32[0] = angleUp(0, 0);
	angleUpMatrix.r[0].m128_f32[1] = angleUp(0, 1);
	angleUpMatrix.r[0].m128_f32[2] = angleUp(0, 2);
	angleUpMatrix.r[1].m128_f32[0] = angleUp(1, 0);
	angleUpMatrix.r[1].m128_f32[1] = angleUp(1, 1);
	angleUpMatrix.r[1].m128_f32[2] = angleUp(1, 2);
	angleUpMatrix.r[2].m128_f32[0] = angleUp(2, 0);
	angleUpMatrix.r[2].m128_f32[1] = angleUp(2, 1);
	angleUpMatrix.r[2].m128_f32[2] = angleUp(2, 2);

	const float MIN_FREAM_TIME = 1.0f / 120;
	float fps = 0;
	float frameTime = 0;
	LARGE_INTEGER timeStart;
	LARGE_INTEGER timeEnd;
	LARGE_INTEGER timeFreq;
	// ���C�����[�v�ɓ���O�ɐ��x���擾���Ă���
	if (QueryPerformanceFrequency(&timeFreq) == FALSE) { // ���̊֐���0(FALSE)���A�鎞�͖��Ή�
		return;
	}
	// 1�x�擾���Ă���(����v�Z�p)
	QueryPerformanceCounter(&timeStart);
	modelPathSize = modelPath.size();
	DWORD sleepTime;

	//std::vector<std::pair<std::string, VertexInfo>> indiceContainer;
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
		// ���̎��Ԃ��擾
		QueryPerformanceCounter(&timeEnd);
		// (���̎��� - �O�t���[���̎���) / ���g�� = �o�ߎ���(�b�P��)
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

		//auto k = _swapChain->GetCurrentBackBufferIndex();
		settingImgui->DrawImGUI(_dev, _cmdList);

		// ���z�̈ʒu���X�V
		sunDir = sun->CalculateDirectionFromDegrees(settingImgui->GetSunAngleX(), settingImgui->GetSunAngleY());
		sun->CalculateViewMatrix();
		shadow->SetVPMatrix(sun->GetShadowViewMatrix(), sun->GetProjMatrix()); // sun->GetViewMatrix()
		skyLUTBuffer.sunDirection.x = sunDir.x;
		skyLUTBuffer.sunDirection.y = sunDir.y;
		skyLUTBuffer.sunDirection.z = sunDir.z;
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


		//for (int i = 0; i < strModelNum; ++i)
		//{
		//	DrawLightMap(i, cbv_srv_Size); // draw lightmap
		//	DrawPeraPolygon(i); // draw background polygon
		//    SetSelfShadowLight(i);			
		//	SetSelfShadowSwitch(i);
		//	SetBloomSwitch(i);
		//}
		
		//DrawModel(0, cbv_srv_Size); // draw pmd model

		//for (int i = 1; i < strModelNum; ++i)
		//{
		//	DrawModel4AO(i, cbv_srv_Size); // draw pmd model for AO to hand over Depth, Normal map.
		//}

		//DrawShrinkTextureForBlur(0, cbv_srv_Size); // draw shrink buffer
		//if (settingImgui->GetSSAOBool())
		//{
		//	DrawAmbientOcclusion(0, cbv_srv_Size); // draw AO	
		//}
		//SetFoVSwitch();
		//SetSSAOSwitch();
		//SetBloomColor();
		


		//shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get());
		sun->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		shadow->SetBoneMatrix(resourceManager[1]->GetMappedMatrixPointer());
		shadow->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		air->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get());
		skyLUT->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		sky->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		

		resourceManager[0]->SetSceneInfo(shadow->GetShadowPosMatrix(), shadow->GetShadowPosInvMatrix(), shadow->GetShadowView(), camera->GetDummyCameraPos(), sun->GetDirection());
		for (int i = 0; i < threadNum; i++)
		{
			SetEvent(m_workerBeginRenderFrame[i]);			
		}
		WaitForMultipleObjects(threadNum, m_workerFinishedRenderFrame, TRUE, INFINITE); // DrawBackBuffer�ɂ�����h���[�R�[�����O�ɒu���Ă�fps�͉��P����...
			// SetEvent(m_workerBeginRenderFrame[1]); // Tell each worker to start drawing.
		//WaitForSingleObject(m_workerFinishedRenderFrame[bbIdx], INFINITE);

		
		// air�̃R�s�[�p���\�[�X��Ԃ�UAV�ɖ߂�
		auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		(
			resourceManager[0]->GetAirBuff().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);


		AllKeyBoolFalse();
		DrawBackBuffer(cbv_srv_Size); // draw back buffer and DirectXTK
		_cmdList3->Close();

		//_cmdList2->Close();
		
		//�R�}���h�L���[�̎��s
		ID3D12CommandList* cmdLists[] = { _cmdList.Get(), _cmdList2.Get(), _cmdList3.Get() };
		_cmdQueue->ExecuteCommandLists(3, cmdLists);

		//ID3D12CommandList* cmdLists2[] = { _cmdList2.Get() };
		//_cmdQueue->ExecuteCommandLists(1, cmdLists2);

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
		//_cmdList->Reset(_cmdAllocator.Get(), nullptr);//�R�}���h���X�g���A�V�����R�}���h���X�g���쐬���ꂽ���̂悤�ɏ�����ԂɃ��Z�b�g
		//_cmdAllocator2->Reset();
		
	
		//// update by imgui
		//SetFov();

		//resourceManager[1]->GetMappedMatrix()->world *= XMMatrixRotationY(0.005f);
		//resourceManager->GetMappedMatrix()->world *= XMMatrixTranslation(0,0,0.03f);

		//�t���b�v���ă����_�����O���ꂽ�C���[�W�����[�U�[�ɕ\��
		_swapChain->Present(1, 0);	
		bbIdx = _swapChain->GetCurrentBackBufferIndex();//���݂̃o�b�N�o�b�t�@���C���f�b�N�X�ɂĎ擾
		// ���������݂Ƀ��[�J�[�X���b�h�ɏ�����������̂ł͂Ȃ��A���̃t���[��������ŏ���������悤�ɂ������B

		

		//_gmemory->Commit(_cmdQueue.Get());
	}

	//delete bufferGPLSetting;
	//delete bufferShaderCompile;
	
	delete lightMapGPLSetting;	
	delete lightMapShaderCompile;
	
	delete textureTransporter;

	UnregisterClass(prepareRenderingWindow->GetWNDCCLASSEX().lpszClassName, prepareRenderingWindow->GetWNDCCLASSEX().hInstance);


	delete textureLoader;


	delete settingShaderCompile;
	delete gPLSetting;

	
	//delete prepareRenderingWindow;
	delete aoShaderCompile;
	delete aoGPLSetting;

	delete peraGPLSetting;

	delete peraPolygon;
	delete peraShaderCompile;	

	delete settingImgui;

	//delete bufferSetRootSignature;
	//delete lightMapRootSignature;
	//delete setRootSignature;
	//delete peraSetRootSignature;

	delete fBXPipeline;
}

void D3DX12Wrapper::AllKeyBoolFalse()
{
	inputW = false;
	inputLeft = false;
	inputRight = false;

	inputUp = false;
}

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

		//���\�[�X�o���A�̏����B�ܯ�������ޯ��ޯ̧��..._COMMON��������ԂƂ��錈�܂�B�����color
		D3D12_RESOURCE_BARRIER barrierDescFBX = {};
		barrierDescFBX.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDescFBX.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		if (num == 0)
		{
			barrierDescFBX.Transition.pResource = resourceManager[0]->GetRenderingBuff().Get();
		}
		else if (num == 1)
		{
			barrierDescFBX.Transition.pResource = resourceManager[0]->GetRenderingBuff2().Get();
		}
		barrierDescFBX.Transition.Subresource = 0;
		barrierDescFBX.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierDescFBX.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		//���\�[�X�o���A�F���\�[�X�ւ̕����̃A�N�Z�X�𓯊�����K�v�����邱�Ƃ��h���C�o�[�ɒʒm
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		
		// ���f���`��
		localCmdList->RSSetViewports(1, viewPort);
		localCmdList->RSSetScissorRects(1, rect);

		// resourceManager[0]��rtv,dsv�ɏW�񂵂Ă���B��@�Ƃ��Ă̓C�}�C�`��...
		auto dsvhFBX = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
		dsvhFBX.ptr += num * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);;
		auto handleFBX = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handleFBX.ptr += num * inc;

		localCmdList->OMSetRenderTargets(1, &handleFBX, false, &dsvhFBX);
		localCmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

		//��ʃN���A
		float clearColor[4];
		//if (num == 0)
		//{
			clearColor[0] = 1.0f;
			clearColor[1] = 1.0f;
			clearColor[2] = 1.0f;
			clearColor[3] = 1.0f;
		//}
		//else if (num == 1)
		//{
		//	clearColor[0] = 0.0f;
		//	clearColor[1] = 0.0f;
		//	clearColor[2] = 0.0f;
		//	clearColor[3] = 0.0f;
		//}

		localCmdList->ClearRenderTargetView(handleFBX, clearColor, 0, nullptr);

		int lastSRVSetNum = 0;
		for (int fbxIndex = 0; fbxIndex < modelPath.size(); ++fbxIndex)
		{
			localCmdList->SetGraphicsRootSignature(fBXRootsignature);
			localCmdList->SetPipelineState(fBXPipeline);

			// �L�[���͏����B�����蔻�菈�����܂߂�B
			if (input->CheckKey(DIK_W)) inputW = true;
			if (input->CheckKey(DIK_LEFT)) inputLeft = true;
			if (input->CheckKey(DIK_RIGHT)) inputRight = true;
			if (input->CheckKey(DIK_UP)) inputUp = true;

			if (resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				// start character with idle animation
				resourceManager[fbxIndex]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);

				// ��Switch���ł��Ȃ����H
				// W Key
				if (inputW)
				{
					resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
				}

				// Left Key
				if (inputLeft)
				{
					resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
				}

				// Right Key
				if (inputRight)
				{
					resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
				}


			}

			// ��Switch���ł��Ȃ����H
			// Left Arrow Key
			if (inputLeft && !resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				resourceManager[fbxIndex]->GetMappedMatrix()->world *= rightSpinMatrix;
				connanDirection *= rightSpinMatrix;
				if (num == 0)
				{
					camera->Transform(rightSpinMatrix);
					sun->ChangeSceneMatrix(rightSpinMatrix);
					sky->ChangeSceneMatrix(rightSpinMatrix);
					shadow->SetRotationMatrix(leftSpinMatrix);
				}
			}

			// Right Arrow Key
			if (inputRight && !resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				resourceManager[fbxIndex]->GetMappedMatrix()->world *= leftSpinMatrix;
				connanDirection *= leftSpinMatrix;
				//if (num == 0)
				//{
					camera->Transform(leftSpinMatrix);
					sun->ChangeSceneMatrix(leftSpinMatrix);
					sky->ChangeSceneMatrix(leftSpinMatrix);
					shadow->SetRotationMatrix(rightSpinMatrix);
				//}
			}

			// Up Arrow Key
			if (inputUp && !resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				resourceManager[fbxIndex]->GetMappedMatrix()->world *= angleUpMatrix;
				//connanDirection *= leftSpinMatrix;
				//if (num == 0)
				//{
					sun->ChangeSceneMatrix(XMMatrixInverse(nullptr, angleUpMatrix));
					sky->ChangeSceneMatrix(angleUpMatrix);
				//}
			}

			// W Key
			if (inputW && !resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				// �����蔻�菈��
				collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, num);
				camera->MoveCamera(forwardSpeed, connanDirection);
				shadow->SetMoveMatrix(forwardSpeed, connanDirection);
			}

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
			dHandle.ptr += cbv_srv_Size * 8;

			localCmdList->SetGraphicsRootDescriptorTable(2, dHandle); // Phong Material Parameters(Numdescriptor : 3)
			dHandle.ptr += cbv_srv_Size;
			//localCmdList->DrawInstanced(resourceManager->GetVertexTotalNum(), 1, 0, 0);

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
			int texStartIndex = 3; // �e�N�X�`�����i�[����f�B�X�N���v�^�e�[�u���ԍ��̊J�n�ʒu
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

				//else
				//{
				//	for (int j = textureTableStartIndex; j < 4 + textureTableStartIndex; ++j)
				//	{
				//		localCmdList->SetGraphicsRootDescriptorTable(j, tHandle); // index of texture
				//		tHandle.ptr += buffSize;
				//	}
				//}

				indiceSize = itIndiceFirst->second.indices.size(); // ���T�C�Y�݂̂�array��p�ӂ��Ă݂�
				localCmdList->DrawIndexedInstanced(indiceSize, 1, ofst, 0, 0);
				//dHandle.ptr += cbv_srv_Size;
				ofst += indiceSize;

				//++itIndiceFirst;
				//++itPhonsInfo;

				if (/*fbxIndex != 1*/i + threadNum >= indiceContainerSize)
				{					
					break;
				}
				itIndiceFirst += threadNum;
				itPhonsInfo += threadNum;
				//if (num == 0 && (i + 1) <= textureIndexes[fbxIndex].size())
				//{
					auto itNext = itIndiceFirst - 1;
					ofst += itNext->second.indices.size();
					tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [1]�X���b�h�ŏ������鎟�̊Y�����f���C���f�b�N�X�̃e�N�X�`����
				//}
				//else if (num == 1 && (i + 1) <= textureIndexes[fbxIndex].size())
				//{
				//	auto itNext = itIndiceFirst - 1;
				//	ofst += itNext->second.indices.size();
				//	tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [0]�X���b�h�ŏ������鎟�̊Y�����f���C���f�b�N�X�̃e�N�X�`����
				//}
				
				itMaterialAndTextureName += textureIndexes[fbxIndex][i + 1];
				itMATCnt += textureIndexes[fbxIndex][i + 1];
				textureTableStartIndex = texStartIndex; // init

			}
			//SetEvent(m_workerSyncronize[num]); // end drawing.
			//WaitForMultipleObjects(threadNum, m_workerSyncronize, TRUE, INFINITE);
			//if (num == 0)
			//{
			//	WaitForSingleObject(m_workerSyncronize[1], INFINITE);
			//}
			//else if(num==1)				
			//{
			//	WaitForSingleObject(m_workerSyncronize[0], INFINITE);
			//}
			
			if (num == 0)
			{
				DrawCollider(fbxIndex);
			}
		}

		//// �}�e���A���̃f�B�X�N���v�^�q�[�v�����[�g�V�O�l�`���̃e�[�u���Ƀo�C���h���Ă���
		//// CBV:1��(matrix)�ASRV:4��(colortex, graytex, spa, sph)���ΏہBSetRootSignature.cpp�Q�ƁB
		//auto materialHandle = bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		//auto inc = buffSize;
		//auto materialHInc = inc * 5; // �s��cbv + (material cbv+�e�N�X�`��srv+sph srv+spa srv+toon srv)
		//materialHandle.ptr += inc; // ���̏����̒��O�ɍs��pCBV������ؽĂɃZ�b�g��������
		//unsigned int idxOffset = 0;

		//// (���Ԃ�)DrawIndexedInstanced�ɂ��`��̑O��SRV����̃e�N�X�`���擾���I���Ă��Ȃ��ƃf�[�^���V�F�[�_�[�ɒʂ�Ȃ�
		//// �Ȃ��A���̃p�X�ł̃f�v�X���`��Ɠ����ɓn���Ă��邪�Q�Əo���Ȃ��̂́A���\�[�X��Ԃ�depth_write�̂܂܂�����Ǝv����
		//localCmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // �f�v�X�}�b�v�i�[
		//materialHandle.ptr += inc;
		//localCmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ���C�g�}�b�v�i�[
		//materialHandle.ptr += inc;

		//for (auto m : pmdMaterialInfo[i]->materials)
		//{
		//	localCmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
		//	//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
		//	localCmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:�ʏ�A1:�e

		//	materialHandle.ptr += materialHInc;
		//	idxOffset += m.indiceNum;
		//}


		// color
		barrierDescFBX.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDescFBX.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		localCmdList->Close();

		SetEvent(m_workerFinishedRenderFrame[num]); // end drawing.
	}
}

void D3DX12Wrapper::DrawCollider(int modelNum)
{

	// ���f���`��
	//_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());
	//_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
	//_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
	//_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());


	//auto dsvh = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	//D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();

	//_cmdList->OMSetRenderTargets(1, &handle, false, &dsvh);
	//_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

	////��ʃN���A
	//float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	_cmdList->SetGraphicsRootSignature(collisionRootSignature->GetRootSignature().Get());
	//_cmdList->SetDescriptorHeaps(1, resourceManager[0]->GetSRVHeap().GetAddressOf());

	_cmdList->SetPipelineState(colliderGraphicsPipelineSetting->GetPipelineState().Get());

	//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP/*D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP*/);

	if (modelNum == 0)
	{
		//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
		for (int i = 0; i < collisionManager->GetOBBNum(); ++i)
		{
			_cmdList->IASetVertexBuffers(0, 1, collisionManager->GetBoxVBVs(i));
			// �C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
			_cmdList->IASetIndexBuffer(collisionManager->GetBoxIBVs(i));
			_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);

			//_cmdList->DrawInstanced(8, 1, 0, 0);
		}
	}

	else
	{
		_cmdList->IASetVertexBuffers(0, 1, collisionManager->GetBoxVBV2());
		_cmdList->IASetIndexBuffer(collisionManager->GetCharacterSphereColliderIBVs());
		_cmdList->DrawIndexedInstanced(144, 1, 0, 0, 0);
		//_cmdList->DrawInstanced(26, 1, 0, 0);
	}

	////�f�B�X�N���v�^�q�[�v�ݒ肨��уf�B�X�N���v�^�q�[�v�ƃ��[�g�p�����[�^�̊֘A�t��	
	//_cmdList->SetDescriptorHeaps(1, resourceManager[fbxIndex]->GetSRVHeap().GetAddressOf());

	//auto dHandle = resourceManager[0]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	//_cmdList->SetGraphicsRootDescriptorTable(0, dHandle); // WVP Matrix(Numdescriptor : 1)
	//dHandle.ptr += buffSize * 2;
	//_cmdList->SetGraphicsRootDescriptorTable(1, dHandle); // Phong Material Parameters(Numdescriptor : 3)
}

//void D3DX12Wrapper::DrawLightMap(unsigned int modelNum, UINT buffSize)
//{
//	constexpr uint32_t shadow_difinition = 1024;
//	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadow_difinition, shadow_difinition);
//	_cmdList->RSSetViewports(1, &vp);
//	CD3DX12_RECT rc(0, 0, shadow_difinition, shadow_difinition);
//	_cmdList->RSSetScissorRects(1, &rc);
//
//	auto dsvh = bufferHeapCreator[modelNum]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
//
//	_cmdList->SetPipelineState(lightMapGPLSetting->GetPipelineState().Get());
//	_cmdList->SetGraphicsRootSignature(lightMapRootSignature->GetRootSignature().Get());
//
//	dsvh.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//	_cmdList->OMSetRenderTargets(0, nullptr, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A
//	//��ʃN���A
//	//_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	_cmdList->IASetVertexBuffers(0, 1, viewCreator[modelNum]->GetVbView());
//
//	_cmdList->IASetIndexBuffer(viewCreator[modelNum]->GetIbView());
//
//	_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[modelNum]->GetCBVSRVHeap().GetAddressOf());
//	_cmdList->SetGraphicsRootDescriptorTable
//	(
//		0, // �o�C���h�̃X���b�g�ԍ�
//		bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//	);
//
//	auto materialHandle2 = bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//	auto inc2 = buffSize;
//	auto materialHInc2 = inc2 * 5; // �s��cbv + (material cbv+�e�N�X�`��srv+sph srv+spa srv+toon srv)
//	materialHandle2.ptr += inc2; // ���̏����̒��O�ɍs��pCBV������ؽĂɃZ�b�g��������
//	unsigned int idxOffset2 = 0;
//
//	for (auto m : pmdMaterialInfo[modelNum]->materials)
//	{
//		_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle2);
//		//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
//		_cmdList->DrawIndexedInstanced(m.indiceNum, 1, idxOffset2, 0, 0); // instanceid 0:�ʏ�A1:�e
//
//		materialHandle2.ptr += materialHInc2;
//		idxOffset2 += m.indiceNum;
//	}
//
//	//_cmdList->DrawIndexedInstanced(pmdMaterialInfo[modelNum]->vertNum, 1, 0, 0, 0);
//
//	// ���C�g�}�b�v��Ԃ������ݸ����ޯĂɕύX����
//	D3D12_RESOURCE_BARRIER barrierDesc4LightMap = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetLightMapBuff().Get(),
//		D3D12_RESOURCE_STATE_DEPTH_WRITE,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4LightMap);
//}
//
//void D3DX12Wrapper::DrawPeraPolygon(unsigned int modelNum)
//{
//	//// �}���`�p�X1�p�X��
//
//	D3D12_RESOURCE_BARRIER barrierDesc4Multi = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff().Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Multi);
//
//	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
//	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());
//
//	auto rtvHeapPointer = bufferHeapCreator[modelNum]->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
//	_cmdList->OMSetRenderTargets(1, &rtvHeapPointer, false, /*&dsvh*/nullptr);
//	_cmdList->ClearRenderTargetView(rtvHeapPointer, clearColor, 0, nullptr);
//	_cmdList->SetGraphicsRootSignature(peraSetRootSignature->GetRootSignature().Get());
//	// no need SetDescriptorHeaps, SetGraphicsRootDescriptorTable, because it only needs rendering.
//
//	_cmdList->SetPipelineState(peraGPLSetting->GetPipelineState().Get());
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	_cmdList->IASetVertexBuffers(0, 1, peraPolygon->GetVBView());
//	_cmdList->DrawInstanced(4, 1, 0, 0);
//
//	// ����߽ؿ����ر���ɖ߂�
//	barrierDesc4Multi = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff().Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Multi);
//}
//
//void D3DX12Wrapper::DrawModel(unsigned int modelNum, UINT buffSize)
//{
//	//���\�[�X�o���A�̏����B�ܯ�������ޯ��ޯ̧��..._COMMON��������ԂƂ��錈�܂�B�����color
//	D3D12_RESOURCE_BARRIER BarrierDesc = {};
//	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	BarrierDesc.Transition.pResource = bufferHeapCreator[modelNum]->GetMultipassBuff2().Get();
//	BarrierDesc.Transition.Subresource = 0;
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	//���\�[�X�o���A�F���\�[�X�ւ̕����̃A�N�Z�X�𓯊�����K�v�����邱�Ƃ��h���C�o�[�ɒʒm
//	_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//
//	// normal
//	D3D12_RESOURCE_BARRIER barrierDesc4test = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff3().Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4test);
//
//	// bloom
//	D3D12_RESOURCE_BARRIER barrierDesc4Bloom = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[0].Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Bloom);
//
//
//	// ���f���`��
//	_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());
//	_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
//	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
//	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());
//
//	auto dsvh = bufferHeapCreator[modelNum]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
//	CD3DX12_CPU_DESCRIPTOR_HANDLE handles[3];
//	auto baseH = bufferHeapCreator[modelNum]->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
//	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	uint32_t offset = 1; // start from No.2 RTV
//	for (auto& handle : handles)
//	{
//		handle.InitOffsetted(baseH, incSize * offset);
//		offset += 1;
//	}
//	_cmdList->OMSetRenderTargets(3, handles, false, &dsvh);
//
//	// �����_�[�^�[�Q�b�g�Ɛ[�x�X�e���V��(�����V�F�[�_�[���F���o���Ȃ��r���[)��CPU�L�q�q�n���h����ݒ肵�ăp�C�v���C���ɒ��o�C���h
//	// �Ȃ̂ł��̓��ނ̃r���[�̓}�b�s���O���Ȃ�����
//	//_cmdList->OMSetRenderTargets(2, rtvs/*&handle*/, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A
//
//	//��ʃN���A
//	float clearColor[4];// = { 0.1f, 0.1f, 0.2f, 1.0f };
//	
//	for (int i = 0; i < 4; ++i)
//	{
//		clearColor[i] = SetBackGroundColor(i);
//	}
//	_cmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
//	_cmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
//	clearColor[0] = 0;
//	clearColor[1] = 0;
//	clearColor[2] = 0;
//	_cmdList->ClearRenderTargetView(handles[2], clearColor, 0, nullptr);
//
//	//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// �`�悳��Ă��镡���̃��f����`�悵�Ă���
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[i]->GetVbView());
//
//		//�C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
//		_cmdList->IASetIndexBuffer(viewCreator[i]->GetIbView());
//
//		//�f�B�X�N���v�^�q�[�v�ݒ肨���
//		//�f�B�X�N���v�^�q�[�v�ƃ��[�g�p�����[�^�̊֘A�t��
//		//�����Ń��[�g�V�O�l�`���̃e�[�u���ƃf�B�X�N���v�^���֘A�t��
//		_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[i]->GetCBVSRVHeap().GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable
//		(
//			0, // �o�C���h�̃X���b�g�ԍ�
//			bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		);
//
//		//////�e�L�X�g�̂悤�ɓ����ɓ�̓��^�C�vDH���Z�b�g����ƁA�O���{�ɂ���Ă͋������ω�����B
//		////// ��ڂ̃Z�b�g�ɂ��NS300/H�ł̓��f�����\������Ȃ��Ȃ����B
//		//////_cmdList->SetDescriptorHeaps(1, &materialDescHeap);
//		//////_cmdList->SetGraphicsRootDescriptorTable
//		//////(
//		//////	1, // �o�C���h�̃X���b�g�ԍ�
//		//////	bufferHeapCreator->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		//////);
//
//		// �}�e���A���̃f�B�X�N���v�^�q�[�v�����[�g�V�O�l�`���̃e�[�u���Ƀo�C���h���Ă���
//		// CBV:1��(matrix)�ASRV:4��(colortex, graytex, spa, sph)���ΏہBSetRootSignature.cpp�Q�ƁB
//		auto materialHandle = bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//		auto inc = buffSize;
//		auto materialHInc = inc * 5; // �s��cbv + (material cbv+�e�N�X�`��srv+sph srv+spa srv+toon srv)
//		materialHandle.ptr += inc; // ���̏����̒��O�ɍs��pCBV������ؽĂɃZ�b�g��������
//		unsigned int idxOffset = 0;
//
//		// (���Ԃ�)DrawIndexedInstanced�ɂ��`��̑O��SRV����̃e�N�X�`���擾���I���Ă��Ȃ��ƃf�[�^���V�F�[�_�[�ɒʂ�Ȃ�
//		// �Ȃ��A���̃p�X�ł̃f�v�X���`��Ɠ����ɓn���Ă��邪�Q�Əo���Ȃ��̂́A���\�[�X��Ԃ�depth_write�̂܂܂�����Ǝv����
//		_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // �f�v�X�}�b�v�i�[
//		materialHandle.ptr += inc;
//		_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ���C�g�}�b�v�i�[
//		materialHandle.ptr += inc;
//
//		for (auto m : pmdMaterialInfo[i]->materials)
//		{
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
//			//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
//			_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:�ʏ�A1:�e
//
//			materialHandle.ptr += materialHInc;
//			idxOffset += m.indiceNum;
//		}
//	}
//
//	// Draw Effeksser Animation
//	if (settingImgui->GetEffectOnOffBool())
//	{
//		DrawEffect();
//	}
//
//	// color
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//	// normal
//	barrierDesc4test = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff3().Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4test);
//
//	// bloom
//	barrierDesc4Bloom = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[0].Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Bloom);
//}
//
//
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
//void D3DX12Wrapper::DrawAmbientOcclusion(unsigned int modelNum, UINT buffSize)
//{
//	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
//	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());
//
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		// �f�v�X�}�b�v�p�o�b�t�@�̏�Ԃ�ǂݍ��݉\�ɕς���
//		D3D12_RESOURCE_BARRIER barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
//		(
//			bufferHeapCreator[i]->/*GetDepthMapBuff*/GetDepthBuff().Get(),
//			D3D12_RESOURCE_STATE_DEPTH_WRITE,
//			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//		);
//	}
//
//	// AO renderer status to
//	D3D12_RESOURCE_BARRIER barrierDesc4AO = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetAOBuff().Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4AO);
//		
//	auto baseH = bufferHeapCreator[modelNum]->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
//	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * 6;
//	baseH.ptr += incSize;
//
//	auto dsvh = bufferHeapCreator[modelNum]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
//	dsvh.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * 2;
//	_cmdList->OMSetRenderTargets(1, &baseH, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A
//
//	float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
//	_cmdList->ClearRenderTargetView(baseH, clearColor, 0, nullptr);
//
//	_cmdList->SetGraphicsRootSignature(aoRootSignature->GetRootSignature().Get());
//
//
//	_cmdList->SetPipelineState(aoGPLSetting->GetPipelineState().Get());
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// �`�悳��Ă��镡���̃��f����`�悵�Ă���
//
//	
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[/*modelNum*/i]->GetVbView());
//		//�C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
//		_cmdList->IASetIndexBuffer(viewCreator[/*modelNum*/i]->GetIbView());
//
//		//// ���߽ϯ�߂Ɩ@���}�b�v�A�V�[���s��͂��ꂼ��̃��f���̂��̂𗘗p����B
//
//		_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[/*modelNum*/i]->GetMultipassSRVHeap().GetAddressOf());
//
//		auto srvHandle = bufferHeapCreator[/*modelNum*/i]->GetMultipassSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//		srvHandle.ptr += buffSize * 4; // depthmap
//		_cmdList->SetGraphicsRootDescriptorTable(2, srvHandle);
//		srvHandle.ptr += buffSize * 2; // scene matrix
//		_cmdList->SetGraphicsRootDescriptorTable(0, srvHandle);
//		srvHandle.ptr += buffSize; // normalmap
//		_cmdList->SetGraphicsRootDescriptorTable(3, srvHandle);
//
//		unsigned int idxOffset = 0;
//		//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
//		for (auto m : pmdMaterialInfo[/*modelNum*/i]->materials)
//		{
//			//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
//			_cmdList->DrawIndexedInstanced(m.indiceNum, 1, idxOffset, 0, 0);
//			idxOffset += m.indiceNum;
//		}
//		//_cmdList->DrawIndexedInstanced(pmdMaterialInfo[modelNum]->indicesNum, 1, 0, 0, 0);
//
//	}
//
//	//  AO renderer status to
//	barrierDesc4AO.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	barrierDesc4AO.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	_cmdList->ResourceBarrier(1, &barrierDesc4AO);
//}

void D3DX12Wrapper::DrawBackBuffer(UINT buffSize)
{

	bbIdx = _swapChain->GetCurrentBackBufferIndex();//���݂̃o�b�N�o�b�t�@���C���f�b�N�X�ɂĎ擾
	//auto localCmdList = m_batchSubmit[bbIdx];

	_cmdList3->RSSetViewports(1, viewPort);
	_cmdList3->RSSetScissorRects(1, rect);

	// �ޯ��ޯ̧�ɕ`�悷��
	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂɕύX����
	barrierDesc4BackBuffer = CD3DX12_RESOURCE_BARRIER::Transition
	(
		_backBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);

	// �e���f���̃f�u�X�}�b�v��ǂݍ��݉\��ԂɕύX����
	D3D12_RESOURCE_BARRIER barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
	(
		resourceManager[0]->GetDepthBuff().Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	//D3D12_RESOURCE_BARRIER barrierDesc4DepthMap2 = CD3DX12_RESOURCE_BARRIER::Transition
	//(
	//	resourceManager[0]->GetDepthBuff2().Get(),
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	//);
	_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);
	//_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap2);

	// only bufferHeapCreator[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart() is initialized as backbuffer
	rtvHeapPointer = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapPointer.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList3->OMSetRenderTargets(1, &rtvHeapPointer, false, /*&dsvh*/nullptr);
	//_cmdList3->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

	//float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	_cmdList3->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);

	// �쐬����ø����̗��p����
	_cmdList3->SetGraphicsRootSignature(bBRootsignature);


	//_cmdList3->SetDescriptorHeaps(1, skyLUT->GetSkyLUTRenderingHeap().GetAddressOf());
	//gHandle = skyLUT->GetSkyLUTRenderingHeap()->GetGPUDescriptorHandleForHeapStart();
	//_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle); // sponza�S�̂̃����_�����O����


	// ��
	_cmdList3->SetDescriptorHeaps(1, resourceManager[0]->GetSRVHeap().GetAddressOf());

	gHandle = resourceManager[0]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle); // sponza�S�̂̃����_�����O����

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(1, gHandle); // connan�̃����_�����O����

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(2, gHandle); //  sponza�S�̂̃f�v�X�}�b�v

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(3, gHandle); // connan�̃f�v�X�}�b�v

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(4, gHandle); // Sky

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(5, gHandle); // imgui

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(6, gHandle); // sun

	_cmdList3->SetPipelineState(bBPipeline);

	_cmdList3->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_cmdList3->IASetVertexBuffers(0, 1, peraPolygon->GetVBView());

	_cmdList3->DrawInstanced(4, 1, 0, 0);

	// after all of drawings, DirectXTK drawing is able to be valid.
	//DrawSpriteFont();

	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂ��猳�ɖ߂�
	barrierDesc4BackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc4BackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);

	// �e�f�u�X�}�b�v��[�x�������݉\��ԂɕύX����
	barrierDesc4DepthMap.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrierDesc4DepthMap.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	//barrierDesc4DepthMap2.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//barrierDesc4DepthMap2.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);
	//_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap2);
	//for (int i = 0; i < strModelNum; ++i)
	//{
	//	// �f�v�X�}�b�v�p�o�b�t�@�̏�Ԃ��������݉\�ɖ߂�
	//	auto barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
	//	(
	//		bufferHeapCreator[i]->/*GetDepthMapBuff*/GetDepthBuff().Get(),
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		D3D12_RESOURCE_STATE_DEPTH_WRITE
	//	);

	//	// ���C�g�}�b�v�p�o�b�t�@�̏�Ԃ��������݉\�ɖ߂�
	//	auto barrierDesc4LightMap = CD3DX12_RESOURCE_BARRIER::Transition
	//	(
	//		bufferHeapCreator[i]->GetLightMapBuff().Get(),
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		D3D12_RESOURCE_STATE_DEPTH_WRITE

	//	);

	//	_cmdList->ResourceBarrier(1, &barrierDesc4LightMap);
	//}
}

//void D3DX12Wrapper::DrawModel4AO(unsigned int modelNum, UINT buffSize)
//{
//	//���\�[�X�o���A�̏����B�ܯ�������ޯ��ޯ̧��..._COMMON��������ԂƂ��錈�܂�B�����color
//	D3D12_RESOURCE_BARRIER BarrierDesc = {};
//	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	BarrierDesc.Transition.pResource = bufferHeapCreator[modelNum]->GetMultipassBuff2().Get();
//	BarrierDesc.Transition.Subresource = 0;
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	//���\�[�X�o���A�F���\�[�X�ւ̕����̃A�N�Z�X�𓯊�����K�v�����邱�Ƃ��h���C�o�[�ɒʒm
//	_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//
//	// normal
//	D3D12_RESOURCE_BARRIER barrierDesc4test = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff3().Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4test);
//
//	// bloom
//	D3D12_RESOURCE_BARRIER barrierDesc4Bloom = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[0].Get(),
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//		D3D12_RESOURCE_STATE_RENDER_TARGET
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Bloom);
//
//
//	// ���f���`��
//	_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());
//	_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
//	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
//	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());
//
//	auto dsvh = bufferHeapCreator[modelNum]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
//	CD3DX12_CPU_DESCRIPTOR_HANDLE handles[3];
//	auto baseH = bufferHeapCreator[modelNum]->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
//	auto incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	uint32_t offset = 1; // start from No.2 RTV
//	for (auto& handle : handles)
//	{
//		handle.InitOffsetted(baseH, incSize * offset);
//		offset += 1;
//	}
//	_cmdList->OMSetRenderTargets(3, handles, false, &dsvh);
//
//	// �����_�[�^�[�Q�b�g�Ɛ[�x�X�e���V��(�����V�F�[�_�[���F���o���Ȃ��r���[)��CPU�L�q�q�n���h����ݒ肵�ăp�C�v���C���ɒ��o�C���h
//	// �Ȃ̂ł��̓��ނ̃r���[�̓}�b�s���O���Ȃ�����
//	//_cmdList->OMSetRenderTargets(2, rtvs/*&handle*/, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A
//
//	//��ʃN���A
//	float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
//	_cmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
//	_cmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
//	clearColor[0] = 0;
//	clearColor[1] = 0;
//	clearColor[2] = 0;
//	_cmdList->ClearRenderTargetView(handles[2], clearColor, 0, nullptr);
//
//	//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// �`�悳��Ă��镡���̃��f����`�悵�Ă���
//
//		//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[modelNum]->GetVbView());
//
//		//�C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
//		_cmdList->IASetIndexBuffer(viewCreator[modelNum]->GetIbView());
//
//		//�f�B�X�N���v�^�q�[�v�ݒ肨���
//		//�f�B�X�N���v�^�q�[�v�ƃ��[�g�p�����[�^�̊֘A�t��
//		//�����Ń��[�g�V�O�l�`���̃e�[�u���ƃf�B�X�N���v�^���֘A�t��
//		_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[modelNum]->GetCBVSRVHeap().GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable
//		(
//			0, // �o�C���h�̃X���b�g�ԍ�
//			bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		);
//
//		//////�e�L�X�g�̂悤�ɓ����ɓ�̓��^�C�vDH���Z�b�g����ƁA�O���{�ɂ���Ă͋������ω�����B
//		////// ��ڂ̃Z�b�g�ɂ��NS300/H�ł̓��f�����\������Ȃ��Ȃ����B
//		//////_cmdList->SetDescriptorHeaps(1, &materialDescHeap);
//		//////_cmdList->SetGraphicsRootDescriptorTable
//		//////(
//		//////	1, // �o�C���h�̃X���b�g�ԍ�
//		//////	bufferHeapCreator->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		//////);
//
//		// �}�e���A���̃f�B�X�N���v�^�q�[�v�����[�g�V�O�l�`���̃e�[�u���Ƀo�C���h���Ă���
//		// CBV:1��(matrix)�ASRV:4��(colortex, graytex, spa, sph)���ΏہBSetRootSignature.cpp�Q�ƁB
//		auto materialHandle = bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//		auto inc = buffSize;
//		auto materialHInc = inc * 5; // �s��cbv + (material cbv+�e�N�X�`��srv+sph srv+spa srv+toon srv)
//		materialHandle.ptr += inc; // ���̏����̒��O�ɍs��pCBV������ؽĂɃZ�b�g��������
//		unsigned int idxOffset = 0;
//
//		// (���Ԃ�)DrawIndexedInstanced�ɂ��`��̑O��SRV����̃e�N�X�`���擾���I���Ă��Ȃ��ƃf�[�^���V�F�[�_�[�ɒʂ�Ȃ�
//		// �Ȃ��A���̃p�X�ł̃f�v�X���`��Ɠ����ɓn���Ă��邪�Q�Əo���Ȃ��̂́A���\�[�X��Ԃ�depth_write�̂܂܂�����Ǝv����
//		_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // �f�v�X�}�b�v�i�[
//		materialHandle.ptr += inc;
//		_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ���C�g�}�b�v�i�[
//		materialHandle.ptr += inc;
//
//		for (auto m : pmdMaterialInfo[modelNum]->materials)
//		{
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
//			//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
//			_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:�ʏ�A1:�e
//
//			materialHandle.ptr += materialHInc;
//			idxOffset += m.indiceNum;
//		}
//	
//	// color
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	_cmdList->ResourceBarrier(1, &BarrierDesc);
//
//	// normal
//	barrierDesc4test = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetMultipassBuff3().Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4test);
//
//	// bloom
//	barrierDesc4Bloom = CD3DX12_RESOURCE_BARRIER::Transition
//	(
//		bufferHeapCreator[modelNum]->GetBloomBuff()[0].Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET,
//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//	);
//	_cmdList->ResourceBarrier(1, &barrierDesc4Bloom);
//}
//
//void D3DX12Wrapper::SetFov()
//{
//	projMat = XMMatrixPerspectiveFovLH
//	(		
//		settingImgui->GetFovValue(), // ��p90��
//		static_cast<float>(prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(prepareRenderingWindow->GetWindowWidth()),
//		1.0, // �j�A�\�N���b�v
//		100.0 // �t�@�[�N���b�v
//	);
//
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		pmdMaterialInfo[i]->mapMatrix->proj = projMat;
//		pmdMaterialInfo[i]->mapMatrix4Lightmap->proj = projMat;
//	}
//}
//
//float D3DX12Wrapper::SetBackGroundColor(int rgbaNum)
//{
//	assert(rgbaNum < 4);
//	float colorValue = settingImgui->GetBGColor(rgbaNum);
//	
//	return colorValue;
//
//}
//
//void D3DX12Wrapper::SetSelfShadowLight(int modelNum)
//{
//	float lightVec[3];
//	XMFLOAT3 lightVecFloat3 = { 0.0f,0.0f,0.0f };
//
//	for (int rgbNum = 0; rgbNum < 3; ++rgbNum)
//	{
//		lightVec[rgbNum] = settingImgui->GetLightVector(rgbNum);
//		pmdMaterialInfo[modelNum]->mapMatrix->lightVec[rgbNum] = lightVec[rgbNum];
//	}
//
//	lightVecFloat3.x = lightVec[0];
//	lightVecFloat3.y = lightVec[1];
//	lightVecFloat3.z = lightVec[2];
//
//	light = XMLoadFloat3(&lightVecFloat3);
//	light = targetPos + XMVector3Normalize(light) * XMVector3Length(XMVectorSubtract(targetPos, eyePos)).m128_f32[0];	
//	pmdMaterialInfo[modelNum]->mapMatrix->lightCamera = XMMatrixLookAtLH(light, targetPos, upVec) * XMMatrixOrthographicLH(40, 40, 1.0f, 100.0f);
//}
//
//void D3DX12Wrapper::SetSelfShadowSwitch(int modelNum)
//{
//	pmdMaterialInfo[modelNum]->mapMatrix->isSelfShadow = settingImgui->GetShadowmapOnOffBool();
//}
//
//void D3DX12Wrapper::SetBloomSwitch(int modelNum)
//{
//	mappingExecuter[modelNum]->GetMappedPostSetting()->isBloom = settingImgui->GetBloomOnOffBool();
//}
//
//void D3DX12Wrapper::SetFoVSwitch()
//{
//	mappingExecuter[0]->GetMappedPostSetting()->isFoV = settingImgui->GetFoVBool();
//}
//
//void D3DX12Wrapper::SetSSAOSwitch()
//{
//	mappingExecuter[0]->GetMappedPostSetting()->isSSAO = settingImgui->GetSSAOBool();
//}
//
//void D3DX12Wrapper::SetBloomColor()
//{
//	for (int i = 0; i < 3; ++i)
//	{
//		mappingExecuter[0]->GetMappedPostSetting()->bloomCol[i] = settingImgui->GetBloomValue(i);
//	}
//}
//
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

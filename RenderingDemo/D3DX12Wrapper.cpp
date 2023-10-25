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

D3DX12Wrapper& D3DX12Wrapper::Instance()
{
	static D3DX12Wrapper instance;
	return instance;
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
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
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

	// �����_�����O�E�B���h�E�ݒ�
	prepareRenderingWindow = new PrepareRenderingWindow;
	prepareRenderingWindow->CreateAppWindow();

	// TextureLoader�N���X�̃C���X�^���X��
	textureLoader = new TextureLoader;

	// �����_�����O�E�B���h�E�\��
	ShowWindow(prepareRenderingWindow->GetHWND(), SW_SHOW);

	// �r���[�|�[�g�ƃV�U�[�̈�̐ݒ�
	prepareRenderingWindow->SetViewportAndRect();

	// �L�[�{�[�h���͊Ǘ��N���X
	input = new Input(prepareRenderingWindow);

	//// ����߽�֘A�׽�Q
	peraLayout = new PeraLayout;
	peraGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/); //TODO PeraLayout,VertexInputLayout�׽�̊��N���X������Ă���ɑΉ�������
	peraPolygon = new PeraPolygon;
	peraSetRootSignature = new PeraSetRootSignature;
	peraShaderCompile = new PeraShaderCompile;
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
	collisionShaderCompile = new CollisionShaderCompile;

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
		(IDXGISwapChain1**)_swapChain.ReleaseAndGetAddressOf());
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
	
	// connan, zig   zig���ɓǂݍ��ނƖ��Ȃ����A��œǂݍ��ނ�D3D12_GPU_DESCRIPTOR_HANDLE�̓ǂݎ��G���[����������B
	// connan, battlefield   battlefield���ɓǂݍ��ނƖ��Ȃ����A��œǂݍ��ނ�D3D12_GPU_DESCRIPTOR_HANDLE�̓ǂݎ��G���[����������B
	// zig, battlefield   battlefield���ɓǂݍ��ނƖ��Ȃ����A��œǂݍ��ނ�D3D12_GPU_DESCRIPTOR_HANDLE�̓ǂݎ��G���[����������B
	// battlefield, battlefield�́Z�ANewConnan,NewConnan�́Z
	// ���e�N�X�`����DrawFBX��SetGraphicsRootDescriptorTable�Ńo�C���h���Ă��邪�A�ʂ̃��b�V����`�悷��ꍇ��SetDescriptorHeaps�ł��̃��b�V����SRV�ɐ؂�ւ��邪�A
	// ���̃��b�V���ɗ��p���Ă���e�N�X�`���������O��`�悵�����b�V���̃e�N�X�`��������菭�Ȃ��ƁASetGraphicsRootDescriptorTable�ɂ��o�C���h�̏㏑��(����Ă���Ɖ���)��
	// �s��ꂸ�O��̃o�C���h���������c���Ă��܂��A#554 DescriptorHeap Invalid�G���[���������Ă���͗l�B�e�N�X�`���̃o�C���h���R�����g�A�E�g�ŃG���[�������邱�ƁA
	// �G���[���b�Z�[�W(���݃o�C���h����Ă���Descriptorheap�����ݐݒ肳��Ă�����̂ƈقȂ�I��)���琄�������B
	// ����̉����́A�ǂݍ��ރ��f�����e�N�X�`���̏��Ȃ����ɂ��邱�ƂɌ�����B
	

	// 0 texture model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_fixed.fbx");
	modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_Test.fbx");
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Boxs_diagonal.fbx"); 
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Box_diagonal_lpos.fbx"); 
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\Box_diagonal.fbx"); 
	// 3 texture model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\ancient\\ziggurat_test2.fbx");
	 
	// 4 textures model
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\NewConnan\\NewConnan2.fbx");
	modelPath.push_back("C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\NewConnan_ZDir.fbx");
	
	//modelPath.push_back("C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_fixed.fbx");
	
	resourceManager.resize(modelPath.size());

	for (int i = 0; i < modelPath.size(); ++i)
	{
		// FBXInfoManager Instance
		fbxInfoManager = FBXInfoManager::Instance();
		fbxInfoManager.Init(modelPath[i]);

		// FBX resource creation
		resourceManager[i] = new ResourceManager(_dev, &fbxInfoManager, prepareRenderingWindow);
		resourceManager[i]->Init();
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
	// _vsBlob��_psBlob�ɼ���ް���ِ߲ݒ�����蓖�Ă�B���ꂼ��̧���߽��ێ����邪�ǂݍ��ݎ��s������nullptr���Ԃ��Ă���B
	auto blobs = settingShaderCompile->SetShaderCompile(setRootSignature, _vsBlob, _psBlob);
	if (blobs.first == nullptr or blobs.second == nullptr) return false;
	_vsBlob = blobs.first;
	_psBlob = blobs.second;	
	delete settingShaderCompile;

	// ����߽1���ڗp
	auto mBlobs = peraShaderCompile->SetPeraShaderCompile(peraSetRootSignature, _vsMBlob, _psMBlob);
	if (mBlobs.first == nullptr or mBlobs.second == nullptr) return false;
	_vsMBlob = mBlobs.first;
	_psMBlob = mBlobs.second;
	delete peraShaderCompile;

	// �R���C�_�[�p
	auto colliderBlobs = collisionShaderCompile->SetPeraShaderCompile(collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);
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

// ����������6�F�R�}���h���X�g�̃N���[�Y(�R�}���h���X�g�̎��s�O�ɂ͕K���N���[�Y����)
	//cmdList->Close();

// ����������7�F�e�o�b�t�@�[���쐬���Ē��_����ǂݍ���
	//for (int i = 0; i < strModelNum; ++i)
	//{
	
	////���_�o�b�t�@�[�̍쐬(���\�[�X�ƈÖٓI�ȃq�[�v�̍쐬) 
	//result = bufferHeapCreator[i]->CreateBufferOfVertex(_dev);

	////�C���f�b�N�X�o�b�t�@�[���쐬(���\�[�X�ƈÖٓI�ȃq�[�v�̍쐬)
	//result = bufferHeapCreator[i]->CreateBufferOfIndex(_dev);

	////�f�v�X�o�b�t�@�[���쐬
	//result = bufferHeapCreator[i]->CreateBufferOfDepthAndLightMap(_dev);

	
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
	//	ID3D12Fence* _fence = nullptr;
	//	UINT64 _fenceVal = 0;
	//	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

// ����������10�F�C�x���g�n���h���̍쐬
// ����������11�FGPU�̏��������҂�

//// Imgui�Ǝ��̏����ݒ�
//	settingImgui = new SettingImgui;
//
//	if (FAILED(settingImgui->Init(_dev, prepareRenderingWindow)))
//	{
//		return false;
//	}	
//
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		bufferHeapCreator[i]->CreateBuff4Imgui(_dev, settingImgui->GetPostSettingSize());
//		viewCreator[i]->CreateCBV4ImguiPostSetting(_dev);
//		mappingExecuter[i]->MappingPostSetting();
//	}

//// DirectXTK�Ǝ��̏����ݒ�
//	DirectXTKInit();
//	
	return true;
}

void D3DX12Wrapper::Run() {
	MSG msg = {};
	auto cbv_srv_Size = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
	auto idn = leftSpinMatrix * rightSpinMatrix;

	//��eigen test
	Matrix3d leftSpinEigen;
	Vector3d axis;
	axis << 0, 1, 0;  //y�����w��
	leftSpinEigen = AngleAxisd(M_PI*0.03f, axis);  //Z�������90�x�����v���ɉ�]
	leftSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	leftSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	leftSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	leftSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	leftSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	leftSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	leftSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	leftSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	leftSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);

	leftSpinEigen = AngleAxisd(-M_PI*0.03f, axis);  //Z�������90�x�����v���ɉ�]
	rightSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	rightSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	rightSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	rightSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	rightSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	rightSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	rightSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	rightSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	rightSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);
	
	while (true)
	{
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
		//settingImgui->DrawDateOfImGUI(_dev, _cmdList, bufferHeapCreator[0]->GetImguiBuff(), bufferHeapCreator[0], k);

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
				
		DrawFBX(cbv_srv_Size);
		AllKeyBoolFalse();
		DrawBackBuffer(cbv_srv_Size); // draw back buffer and DirectXTK

		//�R�}���h���X�g�̃N���[�Y(�R�}���h���X�g�̎��s�O�ɂ͕K���N���[�Y����)
		_cmdList->Close();

		//�R�}���h�L���[�̎��s
		ID3D12CommandList* cmdLists[] = { _cmdList.Get() };
		_cmdQueue->ExecuteCommandLists(1, cmdLists);

		//ID3D12Fence��Signal��CPU���̃t�F���X�ő������s
		//ID3D12CommandQueue��Signal��GPU���̃t�F���X��
		//�R�}���h�L���[�ɑ΂��鑼�̂��ׂĂ̑��삪����������Ƀt�F���X�X�V
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

		_cmdAllocator->Reset();//�R�}���h �A���P�[�^�[�Ɋ֘A�t�����Ă��郁�������ė��p����		
		_cmdList->Reset(_cmdAllocator.Get(), nullptr);//�R�}���h���X�g���A�V�����R�}���h���X�g���쐬���ꂽ���̂悤�ɏ�����ԂɃ��Z�b�g
	
		//// update by imgui
		//SetFov();

		//resourceManager[1]->GetMappedMatrix()->world *= XMMatrixRotationY(0.005f);
		//resourceManager->GetMappedMatrix()->world *= XMMatrixTranslation(0,0,0.03f);

		//�t���b�v���ă����_�����O���ꂽ�C���[�W�����[�U�[�ɕ\��
		_swapChain->Present(1, 0);	

		//_gmemory->Commit(_cmdQueue.Get());
	}

	delete bufferGPLSetting;
	delete bufferShaderCompile;
	
	delete lightMapGPLSetting;	
	delete lightMapShaderCompile;
	
	delete textureTransporter;

	UnregisterClass(prepareRenderingWindow->GetWNDCCLASSEX().lpszClassName, prepareRenderingWindow->GetWNDCCLASSEX().hInstance);


	delete textureLoader;


	delete settingShaderCompile;
	delete gPLSetting;

	
	delete prepareRenderingWindow;
	delete aoShaderCompile;
	delete aoGPLSetting;

	delete peraGPLSetting;
	delete peraLayout;
	delete peraPolygon;
	delete peraShaderCompile;	

	delete settingImgui;

	//delete bufferSetRootSignature;
	//delete lightMapRootSignature;
	//delete setRootSignature;
	//delete peraSetRootSignature;
}

void D3DX12Wrapper::AllKeyBoolFalse()
{
	inputW = false;
	inputLeft = false;
	inputRight = false;
}

void D3DX12Wrapper::DrawFBX(UINT buffSize)
{
	//���\�[�X�o���A�̏����B�ܯ�������ޯ��ޯ̧��..._COMMON��������ԂƂ��錈�܂�B�����color
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = resourceManager[0]->GetRenderingBuff().Get();
	BarrierDesc.Transition.Subresource = 0;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//���\�[�X�o���A�F���\�[�X�ւ̕����̃A�N�Z�X�𓯊�����K�v�����邱�Ƃ��h���C�o�[�ɒʒm
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	// ���f���`��
	/*_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());*/
	/*_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());*/
	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());


	auto dsvh = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();

	_cmdList->OMSetRenderTargets(1, &handle, false, &dsvh);
	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

	//��ʃN���A
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	int lastSRVSetNum = 0;
	for (int fbxIndex = 0; fbxIndex < modelPath.size(); ++fbxIndex)
	{
		_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
		_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());

		// �L�[���͏����B�����蔻�菈�����܂߂�B
		if (input->CheckKey(DIK_W)) inputW = true;
		if (input->CheckKey(DIK_LEFT)) inputLeft = true;
		if (input->CheckKey(DIK_RIGHT)) inputRight = true;

		if (resourceManager[fbxIndex]->GetIsAnimationModel())
		{
			// start character with idle animation
			resourceManager[fbxIndex]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);

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

			// W Key
			if (inputW)
			{
				resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
			}
		}

		// Left Key
		if (inputLeft && !resourceManager[fbxIndex]->GetIsAnimationModel())
		{
			resourceManager[fbxIndex]->GetMappedMatrix()->world *= rightSpinMatrix;
			connanDirection *= rightSpinMatrix;
		}

		// Right Key
		if (inputRight && !resourceManager[fbxIndex]->GetIsAnimationModel())
		{
			resourceManager[fbxIndex]->GetMappedMatrix()->world *= leftSpinMatrix;
			connanDirection *= leftSpinMatrix;
		}

		// W Key
		if (inputW && !resourceManager[fbxIndex]->GetIsAnimationModel())
		{
			// �����蔻�菈��
			collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, fbxIndex);
		}

		//�v���~�e�B�u�^�Ɋւ�����ƁA���̓A�Z���u���[�X�e�[�W�̓��̓f�[�^���L�q����f�[�^�������o�C���h
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/);

		//���_�o�b�t�@�[��CPU�L�q�q�n���h����ݒ�
		_cmdList->IASetVertexBuffers(0, 1, resourceManager[fbxIndex]->GetVbView());

		//���C���f�b�N�X�o�b�t�@�[�̃r���[��ݒ�
		_cmdList->IASetIndexBuffer(resourceManager[fbxIndex]->GetIbView());

		//�f�B�X�N���v�^�q�[�v�ݒ肨��уf�B�X�N���v�^�q�[�v�ƃ��[�g�p�����[�^�̊֘A�t��	
		_cmdList->SetDescriptorHeaps(1, resourceManager[fbxIndex]->GetSRVHeap().GetAddressOf());

		auto dHandle = resourceManager[fbxIndex]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		_cmdList->SetGraphicsRootDescriptorTable(0, dHandle); // WVP Matrix(Numdescriptor : 1)
		dHandle.ptr += buffSize * 2;
		//_cmdList->SetGraphicsRootDescriptorTable(1, dHandle); // Phong Material Parameters(Numdescriptor : 3)

		//_cmdList->DrawInstanced(resourceManager->GetVertexTotalNum(), 1, 0, 0);

		auto indiceContainer = resourceManager[fbxIndex]->GetIndiceAndVertexInfo();
		auto itIndiceFirst = indiceContainer.begin();
		int ofst = 0;

		auto phongInfos = resourceManager[fbxIndex]->GetPhongMaterialParamertInfo();
		auto itPhonsInfos = phongInfos.begin();
		auto mappedPhong = resourceManager[fbxIndex]->GetMappedPhong();
		auto materialAndTexturenameInfo = resourceManager[fbxIndex]->GetMaterialAndTexturePath();
		auto itMaterialAndTextureName = materialAndTexturenameInfo.begin();
		int itMATCnt = 0;
		int matTexSize = materialAndTexturenameInfo.size();
		D3D12_GPU_DESCRIPTOR_HANDLE tHandle = dHandle;
		tHandle.ptr += buffSize * indiceContainer.size();
		int textureTableStartIndex = 2; // 2 is number of texture memory position in SRV
		for (int i = 0; i < indiceContainer.size(); ++i)
		{
			_cmdList->SetGraphicsRootDescriptorTable(1, dHandle); // Phong Material Parameters(Numdescriptor : 3)
			mappedPhong[i]->diffuse[0] = itPhonsInfos->second.diffuse[0];
			mappedPhong[i]->diffuse[1] = itPhonsInfos->second.diffuse[1];
			mappedPhong[i]->diffuse[2] = itPhonsInfos->second.diffuse[2];
			mappedPhong[i]->ambient[0] = itPhonsInfos->second.ambient[0];
			mappedPhong[i]->ambient[1] = itPhonsInfos->second.ambient[1];
			mappedPhong[i]->ambient[2] = itPhonsInfos->second.ambient[2];
			mappedPhong[i]->emissive[0] = itPhonsInfos->second.emissive[0];
			mappedPhong[i]->emissive[1] = itPhonsInfos->second.emissive[1];
			mappedPhong[i]->emissive[2] = itPhonsInfos->second.emissive[2];
			mappedPhong[i]->bump[0] = itPhonsInfos->second.bump[0];
			mappedPhong[i]->bump[1] = itPhonsInfos->second.bump[1];
			mappedPhong[i]->bump[2] = itPhonsInfos->second.bump[2];
			mappedPhong[i]->specular[0] = itPhonsInfos->second.specular[0];
			mappedPhong[i]->specular[1] = itPhonsInfos->second.specular[1];
			mappedPhong[i]->specular[2] = itPhonsInfos->second.specular[2];
			mappedPhong[i]->reflection[0] = itPhonsInfos->second.reflection[0];
			mappedPhong[i]->reflection[1] = itPhonsInfos->second.reflection[1];
			mappedPhong[i]->reflection[2] = itPhonsInfos->second.reflection[2];
			mappedPhong[i]->transparency = itPhonsInfos->second.transparency;

			if (matTexSize > 0) {
				while (itMaterialAndTextureName->first == itPhonsInfos->first)
				{
					//printf("%s\n", itMaterialAndTextureName->first.c_str());
					_cmdList->SetGraphicsRootDescriptorTable(textureTableStartIndex, tHandle); // index of texture
					tHandle.ptr += buffSize;
					++textureTableStartIndex;
					++itMATCnt;
					//++lastSRVSetNum;
					if (itMATCnt == matTexSize) break;
					++itMaterialAndTextureName;

				}
			}

			//else
			//{
			//	for (int j = textureTableStartIndex; j < 4 + textureTableStartIndex; ++j)
			//	{
			//		_cmdList->SetGraphicsRootDescriptorTable(j, tHandle); // index of texture
			//		tHandle.ptr += buffSize;
			//	}
			//}
			
			_cmdList->DrawIndexedInstanced(itIndiceFirst->second.indices.size(), 1, ofst, 0, 0);
			dHandle.ptr += buffSize;
			ofst += itIndiceFirst->second.indices.size();
			++itIndiceFirst;
			++itPhonsInfos;

			textureTableStartIndex = 2; // init
		}

		DrawCollider(fbxIndex, buffSize);
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
	//_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // �f�v�X�}�b�v�i�[
	//materialHandle.ptr += inc;
	//_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ���C�g�}�b�v�i�[
	//materialHandle.ptr += inc;

	//for (auto m : pmdMaterialInfo[i]->materials)
	//{
	//	_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
	//	//�C���f�b�N�X�t���C���X�^���X�����ꂽ�v���~�e�B�u��`��
	//	_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:�ʏ�A1:�e

	//	materialHandle.ptr += materialHInc;
	//	idxOffset += m.indiceNum;
	//}


	// color
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

}


void D3DX12Wrapper::DrawCollider(int modelNum, UINT buffSize)
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

	auto bbIdx = _swapChain->GetCurrentBackBufferIndex();//���݂̃o�b�N�o�b�t�@���C���f�b�N�X�ɂĎ擾
	
	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer()); // ���͏d�v
	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer()); // ���͏d�v

	// �ޯ��ޯ̧�ɕ`�悷��
	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂɕύX����
	D3D12_RESOURCE_BARRIER barrierDesc4BackBuffer = CD3DX12_RESOURCE_BARRIER::Transition
	(
		_backBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &barrierDesc4BackBuffer);

	// only bufferHeapCreator[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart() is initialized as backbuffer
	auto rtvHeapPointer = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapPointer.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList->OMSetRenderTargets(1, &rtvHeapPointer, false, /*&dsvh*/nullptr);
	//_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	_cmdList->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);

	// �쐬����ø����̗��p����
	_cmdList->SetGraphicsRootSignature(peraSetRootSignature->GetRootSignature().Get());
	_cmdList->SetDescriptorHeaps(1, resourceManager[0]->GetSRVHeap().GetAddressOf());

	auto gHandle = resourceManager[0]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	gHandle.ptr += buffSize;
	_cmdList->SetGraphicsRootDescriptorTable(0, gHandle);
	_cmdList->SetPipelineState(peraGPLSetting->GetPipelineState().Get());

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_cmdList->IASetVertexBuffers(0, 1, peraPolygon->GetVBView());

	_cmdList->DrawInstanced(4, 1, 0, 0);

	// after all of drawings, DirectXTK drawing is able to be valid.
	//DrawSpriteFont();

	// �ޯ��ޯ̧��Ԃ������ݸ����ޯĂ��猳�ɖ߂�
	barrierDesc4BackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc4BackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &barrierDesc4BackBuffer);

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

#pragma once

class D3DX12Wrapper
{
private:
	static D3DX12Wrapper* instance;
	std::array<std::string, 3> strModelPath;
	int strModelNum = 0;
	//std::string strMotionPath = "";//"C:\\Users\\RyoTaka\Documents\\RenderingDemoRebuild\\model\\Motion\\squat2.vmd";
	ComPtr<ID3D12Device> _dev = nullptr;
	ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator2 = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator3 = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList2 = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList3 = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_batchSubmit[2];
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal;
	std::vector<ComPtr<ID3D12Resource>> _backBuffers; // �ܯ�������ޯ��ޯ̧� D3D12_RESOURCE_STATE_COMMON�ɐݒ肷�郋�[���B
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	HRESULT result;

	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	GraphicsPipelineSetting* gPLSetting = nullptr;
	//std::vector<BufferHeapCreator*> bufferHeapCreator;
	TextureTransporter* textureTransporter;
	//std::vector<MappingExecuter*> mappingExecuter;
	//std::vector<ViewCreator*> viewCreator;

	//std::vector<DirectX::TexMetadata*> metaData;
	//std::vector<DirectX::Image*> img;
	//std::vector<DirectX::TexMetadata*> toonMetaData;
	//std::vector<DirectX::Image*> toonImg;



	// �R�s�[�R���X�g���N�^
	D3DX12Wrapper(const D3DX12Wrapper& x) { };

	// ������Z�q
	D3DX12Wrapper& operator=(const D3DX12Wrapper&) { return *this; };

	SetRootSignature* setRootSignature = nullptr;
	SettingShaderCompile* settingShaderCompile = nullptr;
	VertexInputLayout* vertexInputLayout = nullptr;
	//std::vector<PMDMaterialInfo*> pmdMaterialInfo;
	//std::vector<VMDMotionInfo*> vmdMotionInfo;
	//std::vector<PMDActor*> pmdActor;
	PrepareRenderingWindow* prepareRenderingWindow = nullptr;	
	TextureLoader* textureLoader = nullptr;

	//std::map<int, std::vector<DirectX::XMMATRIX>*> boneMatrices;
	//std::map<int, std::map<std::string, BoneNode>> bNodeTable;
	//unsigned int _duration; // �A�j���[�V�����̍ő�t���[���ԍ�

	//void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
	//void UpdateVMDMotion(std::map<std::string, BoneNode> bNodeTable, 
	//std::unordered_map<std::string, std::vector<KeyFrame>> motionData);

	//float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// ����߽�֘A
	PeraGraphicsPipelineSetting* peraGPLSetting = nullptr;
	PeraLayout* peraLayout = nullptr;
	PeraPolygon* peraPolygon = nullptr;
	PeraSetRootSignature* peraSetRootSignature = nullptr;
	/*PeraShaderCompile*/SettingShaderCompile* peraShaderCompile = nullptr;
	//PeraGraphicsPipelineSetting* bufferGPLSetting = nullptr;
	//PeraSetRootSignature* bufferSetRootSignature = nullptr;

	//BufferShaderCompile* bufferShaderCompile = nullptr;

	// ���C�g�}�b�v�֘A
	LightMapGraphicsPipelineSetting* lightMapGPLSetting = nullptr;
	SetRootSignature* lightMapRootSignature = nullptr;
	LightMapShaderCompile* lightMapShaderCompile = nullptr;
	ComPtr<ID3D10Blob> _lightMapVSBlob = nullptr; // ���C�g�}�b�v�p���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _lightMapPSBlob = nullptr; // ���C�g�}�b�v�p�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	//XMFLOAT4 _planeNormalVec;
	//XMFLOAT3 lightVec;
	
	// bloom	
	PeraGraphicsPipelineSetting* bloomGPLSetting = nullptr;
	PeraSetRootSignature* bloomRootSignature = nullptr;
	BloomShaderCompile* bloomShaderCompile = nullptr;
	ComPtr<ID3D10Blob> _bloomVSBlob = nullptr; // bloom�p���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _bloomPSBlob = nullptr; // bloom�p�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	
	// AO
	AOShaderCompile* aoShaderCompile = nullptr;
	AOGraphicsPipelineSetting* aoGPLSetting = nullptr;
	SetRootSignature* aoRootSignature = nullptr;
	ComPtr<ID3D10Blob> _aoVSBlob = nullptr; // AO�p���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _aoPSBlob = nullptr; // AO�p�s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p

	// Imgui
	SettingImgui* settingImgui = nullptr;
	void SetFov();
	float SetBackGroundColor(int rgbaNum);
	void SetSelfShadowLight(int modelNum);
	void SetSelfShadowSwitch(int modelNum);
	void SetBloomSwitch(int modelNum);
	void SetFoVSwitch();
	void SetSSAOSwitch();
	void SetBloomColor();

	//DirectX::XMVECTOR light;
	//DirectX::XMVECTOR eyePos;
	//DirectX::XMVECTOR targetPos;
	//DirectX::XMVECTOR upVec;

	// Effekseer
	EffekseerRenderer::RendererRef _efkRenderer = nullptr; // effect renderer
	Effekseer::ManagerRef _efkManager = nullptr; // effect manager
	Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> _efkMemoryPool = nullptr; // memory pool
	Effekseer::RefPtr<EffekseerRenderer::CommandList> _efkCmdList = nullptr; // for DirectX12, Vulkan
	Effekseer::EffectRef _effect = nullptr; // entity of effect(effect file)
	Effekseer::Handle _efkHandle; // effect handle(exceuted effect address)
	Effekseer::Matrix44 fkViewMat;
	Effekseer::Matrix44 fkProjMat;
	void DrawEffect();

	// DirectXTK
	GraphicsMemory* _gmemory = nullptr; // ���̨�����ص�޼ު��
	SpriteFont* _spriteFont = nullptr; // �t�H���g�\���p�I�u�W�F�N�g
	SpriteBatch* _spriteBatch = nullptr; // �X�v���C�g�\���p�I�u�W�F�N�g
	void DirectXTKInit();
	void DrawSpriteFont();

	// draw method
	void DrawLightMap(unsigned int modelNum, UINT buffSize); // draw light map
	void DrawModel(unsigned int modelNum, UINT buffSize); // draw pmd model
	void DrawShrinkTextureForBlur(unsigned int modelNum, UINT buffSize); // draw blur texture
	void DrawPeraPolygon(unsigned int modelNum); // draw background polygon	
	void DrawAmbientOcclusion(unsigned int modelNum, UINT buffSize); // draw ambient occlusion
	void DrawBackBuffer(UINT buffSize); // draw back buffers
	void DrawModel4AO(unsigned int modelNum, UINT buffSize);

	// Matrix
	XMMATRIX projMat;

	// Rebuild
	/*FBXInfoManager fbxInfoManager;*/
	std::vector<ResourceManager*> resourceManager;
	CollisionManager* collisionManager = nullptr;
	ColliderGraphicsPipelineSetting* colliderGraphicsPipelineSetting = nullptr;
	CollisionRootSignature* collisionRootSignature = nullptr;
	SettingShaderCompile* collisionShaderCompile = nullptr;
	ComPtr<ID3D10Blob> _vsCollisionBlob = nullptr; // �R���C�_�[�`��p
	ComPtr<ID3D10Blob> _psCollisionBlob = nullptr; // �R���C�_�[�`��p
	//BoundingSphere* characterBSphere = nullptr;
	XMMATRIX connanDirection = XMMatrixIdentity(); // �L�����N�^�[�̉�]���܂߂������̊Ď��ϐ�
	//XMMATRIX connanDirectionUntilCollision = XMMatrixIdentity(); // �L�����N�^�[���Փ˂���܂ł̕����Ď��ϐ��B�Փˏ�Ԃ��甲���o���̂ɗ��p���A�����o���������connanDirection�ōX�V����B

	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	//ComPtr<ID3D12Resource> backBufferResource = nullptr;
	Input* input = nullptr;
	std::vector< std::string> modelPath;
	std::pair<std::string, int> idleMotionDataNameAndMaxFrame;
	std::pair<std::string, int> walkingMotionDataNameAndMaxFrame;
	std::pair<std::string, int> runMotionDataNameAndMaxFrame;
	bool inputRet; // ���͂ɑ΂��锻�� true:�w��̓��͂���
	bool inputW = false;
	bool inputLeft = false;
	bool inputRight = false;
	void AllKeyBoolFalse();

	/*double forwardSpeed = -0.05;*/
	double forwardSpeed = -0.026/* * 15*/;
	double turnSpeed = 5;
	XMMATRIX leftSpinMatrix = XMMatrixIdentity();
	XMMATRIX rightSpinMatrix = XMMatrixIdentity();
	double sneakCorrectNum = 0.049;
	// 
	struct ThreadParameter
	{
		int threadIndex;
	};
	int threadNum = 2;
	ThreadParameter m_threadParameters[2];
	HANDLE m_threadHandles[2];
	// Synchronization objects.
	HANDLE m_workerBeginRenderFrame[2];
	HANDLE m_workerFinishedRenderFrame[2];
	HANDLE m_workerSyncronize[2];
	void LoadContexts();
	void threadWorkTest(int num/*, ComPtr<ID3D12GraphicsCommandList> cmList*/);
	void DrawFBXMulti();
	void DrawFBX();
	// DrawFBX�p�f�[�^
	UINT cbv_srv_Size;
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvhFBX;
	//D3D12_CPU_DESCRIPTOR_HANDLE handleFBX;
	ID3D12RootSignature* fBXRootsignature = nullptr;
	ID3D12PipelineState* fBXPipeline = nullptr;
	std::vector<D3D12_VERTEX_BUFFER_VIEW*> vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW*> ibViews;
	std::vector<ComPtr<ID3D12DescriptorHeap>> srvHeapAddresses;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> dHandles;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>> indiceContainer; // ���ÓI�f�[�^�\���ł���std::array�ւ̒u�������͉\���H	
	std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> itIndiceFirsts;
	//::vector<std::pair<std::string, VertexInfo>>::iterator itIndiceFirst;
	std::vector<std::vector<std::pair<std::string, PhongInfo>>> phongInfos;
	std::vector<std::vector<std::pair<std::string, PhongInfo>>::iterator> itPhonsInfos;
	//std::vector<std::pair<std::string, PhongInfo>>::iterator itPhonsInfo;
	std::vector<std::vector<std::pair<std::string, std::string>>> materialAndTexturenameInfo;
	std::vector<std::vector<std::pair<std::string, std::string>>::iterator> itMaterialAndTextureNames;
	//std::vector<std::pair<std::string, std::string>>::iterator itMaterialAndTextureName;
	std::vector<int> matTexSizes;
	//int ofst = 0;
	//short indiceContainerSize = 0;
	//int itMATCnt = 0;
	//int matTexSize = 0;
	//int textureTableStartIndex = 0;
	//size_t indiceSize = 0;
	//D3D12_GPU_DESCRIPTOR_HANDLE tHandle;
	D3D12_RESOURCE_BARRIER barrierDescFBX = {};

	void DrawCollider(int modelNum);

	// �`��p�C�v���C���ɋ��ʂ��ĕK�v�ȃf�[�^
	short modelPathSize;
	const D3D12_VIEWPORT* viewPort = nullptr;
	const D3D12_RECT* rect = nullptr;

	// DrawBackBuffer�ŗ��p����
	UINT bbIdx;
	ID3D12RootSignature* bBRootsignature = nullptr;
	ID3D12PipelineState* bBPipeline = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapPointer;
	D3D12_GPU_DESCRIPTOR_HANDLE gHandle;
	D3D12_RESOURCE_BARRIER barrierDesc4BackBuffer = {};
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };

	// Sky
	SkyLUT* skyLUT = nullptr;
	ParticipatingMedia participatingMedia;
	ParticipatingMedia calculatedParticipatingMedia;
	SkyLUTBuffer skyLUTBuffer;

	//�������R���C�_�[debug�p
	int debugNum = 2;



public:

	D3DX12Wrapper();

	///Application�̃V���O���g���C���X�^���X�𓾂�
	static D3DX12Wrapper& Instance();

	static D3DX12Wrapper* GetInstance() { return instance; };

	/// <summary>
	/// �e��f�o�C�X�̍쐬 
	/// </summary>
	/// <returns></returns>
	HRESULT D3DX12DeviceInit();

	// �`��̈�Ȃǂ̏�����
	bool PrepareRendering();

	// �p�C�v���C��������
	bool PipelineInit();

	// effekseer initialize
	void EffekseerInit();

	/// <summary>
	/// �e�탊�\�[�X�̏�����
	/// </summary>
	/// <returns></returns>
	bool ResourceInit();

	///���[�v�N��
	void Run();

	///�㏈��
	void Terminate();

	//�f�X�g���N�^
	~D3DX12Wrapper();

};

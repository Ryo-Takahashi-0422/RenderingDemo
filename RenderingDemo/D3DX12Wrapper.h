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
	std::vector<ComPtr<ID3D12Resource>> _backBuffers; // ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧｰ D3D12_RESOURCE_STATE_COMMONに設定するルール。
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	HRESULT result;

	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	GraphicsPipelineSetting* gPLSetting = nullptr;
	TextureTransporter* textureTransporter;

	// コピーコンストラクタ
	D3DX12Wrapper(const D3DX12Wrapper& x) { };

	// 代入演算子
	D3DX12Wrapper& operator=(const D3DX12Wrapper&) { return *this; };

	SetRootSignature* setRootSignature = nullptr;
	SettingShaderCompile* settingShaderCompile = nullptr;
	VertexInputLayout* vertexInputLayout = nullptr;
	PrepareRenderingWindow* prepareRenderingWindow = nullptr;	
	TextureLoader* textureLoader = nullptr;

	// ﾏﾙﾁﾊﾟｽ関連
	PeraGraphicsPipelineSetting* peraGPLSetting = nullptr;
	PeraLayout* peraLayout = nullptr;
	PeraPolygon* peraPolygon = nullptr;
	PeraSetRootSignature* peraSetRootSignature = nullptr;
	/*PeraShaderCompile*/SettingShaderCompile* peraShaderCompile = nullptr;

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
	GraphicsMemory* _gmemory = nullptr; // ｸﾞﾗﾌｨｯｸｽﾒﾓﾘｵﾌﾞｼﾞｪｸﾄ
	SpriteFont* _spriteFont = nullptr; // フォント表示用オブジェクト
	SpriteBatch* _spriteBatch = nullptr; // スプライト表示用オブジェクト
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
	OBBManager* oBBManager = nullptr;
	//BoundingSphere* characterBSphere = nullptr;
	XMMATRIX connanDirection = XMMatrixIdentity(); // キャラクターの回転も含めた方向の監視変数
	//XMMATRIX connanDirectionUntilCollision = XMMatrixIdentity(); // キャラクターが衝突するまでの方向監視変数。衝突状態から抜け出すのに利用し、抜け出した直後にconnanDirectionで更新する。
	Camera* camera = nullptr;

	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	//ComPtr<ID3D12Resource> backBufferResource = nullptr;
	Input* input = nullptr;
	std::vector< std::string> modelPath;
	std::pair<std::string, int> idleMotionDataNameAndMaxFrame;
	std::pair<std::string, int> walkingMotionDataNameAndMaxFrame;
	std::pair<std::string, int> runMotionDataNameAndMaxFrame;
	bool inputRet; // 入力に対する判定 true:指定の入力あり
	bool inputW = false;
	bool inputLeft = false;
	bool inputRight = false;
	bool inputUp = false;
	void AllKeyBoolFalse();

	/*double forwardSpeed = -0.05;*/
	double forwardSpeed = -0.026/* * 15*/;
	double turnSpeed = 5;
	XMMATRIX leftSpinMatrix = XMMatrixIdentity();
	XMMATRIX rightSpinMatrix = XMMatrixIdentity();
	XMMATRIX angleUpMatrix = XMMatrixIdentity();
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
	void threadWorkTest(int num);

	// DrawFBX用データ
	UINT cbv_srv_Size;
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvhFBX;
	//D3D12_CPU_DESCRIPTOR_HANDLE handleFBX;
	ID3D12RootSignature* fBXRootsignature = nullptr;
	ID3D12PipelineState* fBXPipeline = nullptr;
	std::vector<D3D12_VERTEX_BUFFER_VIEW*> vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW*> ibViews;
	std::vector<ComPtr<ID3D12DescriptorHeap>> srvHeapAddresses;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> dHandles;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>> indiceContainer; // ★静的データ構造であるstd::arrayへの置き換えは可能か？	
	std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> itIndiceFirsts;
	//::vector<std::pair<std::string, VertexInfo>>::iterator itIndiceFirst;
	std::vector<std::vector<std::pair<std::string, PhongInfo>>> phongInfos;
	std::vector<std::vector<std::pair<std::string, PhongInfo>>::iterator> itPhonsInfos;
	//std::vector<std::pair<std::string, PhongInfo>>::iterator itPhonsInfo;
	std::vector<std::vector<std::pair<std::string, std::string>>> materialAndTexturenameInfo;
	std::vector<std::vector<std::pair<std::string, std::string>>::iterator> itMaterialAndTextureNames;
	std::map<int, std::vector<int>> textureIndexes;
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
	XMFLOAT3 charaPos = { 0,0,0 };
	// 描画パイプラインに共通して必要なデータ
	short modelPathSize;
	const D3D12_VIEWPORT* viewPort = nullptr;
	const D3D12_RECT* rect = nullptr;

	// DrawBackBufferで利用する
	UINT bbIdx;
	ID3D12RootSignature* bBRootsignature = nullptr;
	ID3D12PipelineState* bBPipeline = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapPointer;
	D3D12_GPU_DESCRIPTOR_HANDLE gHandle;
	D3D12_RESOURCE_BARRIER barrierDesc4BackBuffer = {};
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };

	// Sky関連
	Sky* sky = nullptr;
	SkyLUT* skyLUT = nullptr;
	ShadowFactor* shadowFactor = nullptr;
	ParticipatingMedia participatingMedia;
	ParticipatingMedia calculatedParticipatingMedia;
	SkyLUTBuffer skyLUTBuffer;
	Sun* sun = nullptr;	
	Shadow* shadow = nullptr;
	Air* air = nullptr;
	Blur* shadowRenderingBlur = nullptr;
	Blur* colorIntegraredBlur = nullptr;
	Blur* ssaoBlur = nullptr;
	//Blur* airBlur = nullptr;
	ComputeBlur* comBlur = nullptr;
	Integration* integration = nullptr;
	DepthMapIntegration* depthMapIntegration = nullptr;
	CalculateSSAO* calculateSSAO = nullptr;
	
	//★★★コライダーdebug用
	int debugNum = 2;



public:

	D3DX12Wrapper();

	///Applicationのシングルトンインスタンスを得る
	static D3DX12Wrapper& Instance();

	static D3DX12Wrapper* GetInstance() 
	{ 
		/*return instance; */
		static D3DX12Wrapper instance;        // スタティク変数として Singleton オブジェクトを生成
		return &instance;
	};

	/// <summary>
	/// 各種デバイスの作成 
	/// </summary>
	/// <returns></returns>
	HRESULT D3DX12DeviceInit();

	// 描画領域などの初期化
	bool PrepareRendering();

	// パイプライン初期化
	bool PipelineInit();

	// effekseer initialize
	void EffekseerInit();

	/// <summary>
	/// 各種リソースの初期化
	/// </summary>
	/// <returns></returns>
	bool ResourceInit();

	///ループ起動
	void Run();

	///後処理
	void Terminate();

	//デストラクタ
	~D3DX12Wrapper();

};

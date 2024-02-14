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

// 後処理
void D3DX12Wrapper::Terminate()
{

};

D3DX12Wrapper::~D3DX12Wrapper()
{

};

HRESULT D3DX12Wrapper::D3DX12DeviceInit()
{
	//result = CoInitializeEx(0, COINIT_MULTITHREADED);

	//ファクトリーの生成
	result = S_OK;
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()))))
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()))))
		{
			return -1;
		}
	}

	//グラボが複数挿入されている場合にアダプターを選択するための処理
	//アダプターの列挙用
	std::vector <IDXGIAdapter*> adapters;
	//特定の名前を持つアダプターオブジェクトが入る
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
		adpt->GetDesc(&adesc); //アダプターの説明オブジェクト取得

		//探したいアダプターの名前を確認
		std::wstring strDesc = adesc.Description;
		//printf("%ls\n", strDesc.c_str());
		//printf("%x\n", adesc.DeviceId);

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//フィーチャレベル列挙
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//Direct3D デバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter.Get(), lv, IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel = lv;
			break;//生成可能なバージョンが見つかったらループ中断
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

	// SetRootSignatureBaseクラスのインスタンス化
	setRootSignature = new SetRootSignature;	

	// SettingShaderCompileクラスのインスタンス化
	settingShaderCompile = new SettingShaderCompile;

	// VertexInputLayoutクラスのインスタンス化
	vertexInputLayout = new VertexInputLayout;

	// GraphicsPipelineSettingクラスのインスタンス化
	gPLSetting = new GraphicsPipelineSetting(vertexInputLayout);
	delete vertexInputLayout;

	// レンダリングウィンドウ設定
	prepareRenderingWindow = new PrepareRenderingWindow;
	prepareRenderingWindow->CreateAppWindow();

	// TextureLoaderクラスのインスタンス化
	textureLoader = new TextureLoader;

	//// レンダリングウィンドウ表示
	//ShowWindow(prepareRenderingWindow->GetHWND(), SW_SHOW);

	// ビューポートとシザー領域の設定
	prepareRenderingWindow->SetViewportAndRect();

	// キーボード入力管理クラス
	input = new Input(prepareRenderingWindow);

	//// ﾏﾙﾁﾊﾟｽ関連ｸﾗｽ群
	peraLayout = new PeraLayout;
	peraGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/); //TODO PeraLayout,VertexInputLayoutｸﾗｽの基底クラスを作ってそれに対応させる
	peraPolygon = new PeraPolygon;
	peraSetRootSignature = new PeraSetRootSignature;
	peraShaderCompile = new SettingShaderCompile;
	//bufferGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/);
	//bufferSetRootSignature = new PeraSetRootSignature;
	//bufferShaderCompile = new BufferShaderCompile;

	//// ライトマップ関連
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

	////デバイス取得
	//auto hdc = GetDC(prepareRenderingWindow->GetHWND());
	//auto rate = GetDeviceCaps(hdc, VREFRESH);

	return true;
}

bool D3DX12Wrapper::PipelineInit(){
//●パイプライン初期化　処理１～７
//初期化処理１：デバッグレイヤーをオンに
#ifdef _DEBUG
	Utility::EnableDebugLayer();
#endif
//初期化処理２：各種デバイスの初期設定
	D3DX12DeviceInit();

	// カラークリアに関するWarningをフィルタリング(メッセージが埋もれてしまう...)
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
		// ついでにエラーメッセージでブレークさせる
		result = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	}

//初期化処理３：コマンドキューの記述用意・作成

	//コマンドキュー生成、詳細obj生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウト無し
	cmdQueueDesc.NodeMask = 0;//アダプターを一つしか使わないときは0でOK
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//コマンドキューの優先度
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//コマンドリストと合わせる

	//コマンドキュー生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

//初期化処理４：スワップチェーンの生成
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
	result = _dxgiFactory->CreateSwapChainForHwnd( //ここで生成
		_cmdQueue.Get(),
		prepareRenderingWindow->GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapChain./*ReleaseAnd*/GetAddressOf());
	if (result != S_OK) return false;

//初期化処理５：レンダーターゲットビュー(RTV)の記述子ヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

//初期化処理６：フレームリソース(各フレームのレンダーターゲットビュー)を作成
	_backBuffers.resize(swapChainDesc.BufferCount); // ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧｰのﾘｻｲｽﾞ
	handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();//ヒープの先頭を表す CPU 記述子ハンドルを取得

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int idx = 0; idx < swapChainDesc.BufferCount; idx++)
	{   //swapEffect DXGI_SWAP_EFFECT_FLIP_DISCARD の場合は最初のバッファーのみアクセス可能
		result = _swapChain->GetBuffer(idx, IID_PPV_ARGS(_backBuffers[idx].ReleaseAndGetAddressOf()));//SWCにバッファーのIIDとそのIIDポインタを教える(SWCがレンダリング時にアクセスする)
		if (result != S_OK) return false;

		_dev->CreateRenderTargetView//リソースデータ(_backBuffers)にアクセスするためのレンダーターゲットビューをhandleアドレスに作成
		(
			_backBuffers[idx].Get(),//レンダーターゲットを表す ID3D12Resource オブジェクトへのポインター
			&rtvDesc,//レンダー ターゲット ビューを記述する D3D12_RENDER_TARGET_VIEW_DESC 構造体へのポインター。
			handle//新しく作成されたレンダーターゲットビューが存在する宛先を表す CPU 記述子ハンドル(ヒープ上のアドレス)
		);

		//handleｱﾄﾞﾚｽを記述子のアドレスを扱う記述子サイズ分オフセットしていく
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

//初期化処理７：コマンドアロケーターを作成
			//コマンドアロケーター生成>>コマンドリスト作成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator2.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator3.ReleaseAndGetAddressOf()));
	if (result != S_OK) return false;

	return true;
}

void D3DX12Wrapper::EffekseerInit()
{
	_efkManager = Effekseer::Manager::Create(8000);
	// DirectXは左手系のため、これに合わせる
	_efkManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

	auto graphicsDevice = EffekseerRendererDX12::CreateGraphicsDevice(_dev.Get(), _cmdQueue.Get(), 2);
	
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
	_efkRenderer = EffekseerRendererDX12::Create(graphicsDevice, &format, 1, DXGI_FORMAT_UNKNOWN, false, 8000);
	_efkMemoryPool = EffekseerRenderer::CreateSingleFrameMemoryPool(_efkRenderer->GetGraphicsDevice());
	_efkCmdList = EffekseerRenderer::CreateCommandList(_efkRenderer->GetGraphicsDevice(), _efkMemoryPool);

	_efkRenderer->SetCommandList(_efkCmdList);

	// 描画モジュールの設定
	_efkManager->SetSpriteRenderer(_efkRenderer->CreateSpriteRenderer());
	_efkManager->SetRibbonRenderer(_efkRenderer->CreateRibbonRenderer());
	_efkManager->SetRingRenderer(_efkRenderer->CreateRingRenderer());
	_efkManager->SetTrackRenderer(_efkRenderer->CreateTrackRenderer());
	_efkManager->SetModelRenderer(_efkRenderer->CreateModelRenderer());

	// テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
	// ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
	_efkManager->SetTextureLoader(_efkRenderer->CreateTextureLoader());
	_efkManager->SetModelLoader(_efkRenderer->CreateModelLoader());
	_efkManager->SetMaterialLoader(_efkRenderer->CreateMaterialLoader());
	_efkManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	// エフェクト自体の設定
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
	//●リソース初期化
	

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
	
	// TextureTransporterクラスのインスタンス化
	textureTransporter = new TextureTransporter;
	
	// 初期化処理1：ルートシグネチャ設定
	if (FAILED(setRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}

	// ﾏﾙﾁﾊﾟｽ用
	if (FAILED(peraSetRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}

	// ｺﾗｲﾀﾞｰ用
	if (FAILED(collisionRootSignature->SetRootsignatureParam(_dev)))
	{
		return false;
	}

	

	//// 表示用
	//if (FAILED(bufferSetRootSignature->SetRootsignatureParam(_dev)))
	//{
	//	return false;
	//}

	//// ライトマップ用
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
	
// 初期化処理2：シェーダーコンパイル設定
    
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _vsMBlob = nullptr; // ﾏﾙﾁﾊﾟｽ用頂点シェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _psMBlob = nullptr; // ﾏﾙﾁﾊﾟｽ用頂点ピクセルシェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _vsBackbufferBlob = nullptr; // 表示用頂点シェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _psBackbufferBlob = nullptr; // 表示用頂点ピクセルシェーダーオブジェクト格納用

	// FBXモデル描画用のシェーダーセッティング
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

	// バックバッファ描画用
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

	// コライダー描画用
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
	
	//// 表示用
	//auto bufferBlobs = bufferShaderCompile->SetPeraShaderCompile(bufferSetRootSignature, _vsBackbufferBlob, _psBackbufferBlob);
	//if (bufferBlobs.first == nullptr or bufferBlobs.second == nullptr) return false;
	//_vsBackbufferBlob = bufferBlobs.first;
	//_psBackbufferBlob = bufferBlobs.second;

	//// ライトマップ用
	//auto lightMapBlobs = lightMapShaderCompile->SetShaderCompile(lightMapRootSignature, _lightMapVSBlob, _lightMapPSBlob);
	//if (lightMapBlobs.first == nullptr) return false;
	//_lightMapVSBlob = lightMapBlobs.first;
	//_lightMapPSBlob = lightMapBlobs.second; // こちらはnullptr

	//// bloom
	//auto bloomBlobs = bloomShaderCompile->SetPeraShaderCompile(bloomRootSignature, _bloomVSBlob, _bloomPSBlob);
	//if (bloomBlobs.first == nullptr or bufferBlobs.second == nullptr) return false;
	//_bloomVSBlob = bloomBlobs.first; // こちらはnullpt
	//_bloomPSBlob = bloomBlobs.second;

	//// AO
	//auto aoBlobs = aoShaderCompile->SetShaderCompile(aoRootSignature, _aoVSBlob, _aoPSBlob);
	//if (aoBlobs.first == nullptr or aoBlobs.second == nullptr) return false;
	//_aoVSBlob = aoBlobs.first; // こちらはnullpt
	//_aoPSBlob = aoBlobs.second;

// 初期化処理3：頂点入力レイアウトの作成及び
// 初期化処理4：パイプライン状態オブジェクト(PSO)のDesc記述してオブジェクト作成
	result = gPLSetting->CreateGPStateWrapper(_dev, setRootSignature, _vsBlob, _psBlob);
	
	// コライダー用
	result = colliderGraphicsPipelineSetting->CreateGPStateWrapper(_dev, collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);

	// ﾊﾞｯｸﾊﾞｯﾌｧ用
	result = peraGPLSetting->CreateGPStateWrapper(_dev, peraSetRootSignature, _vsMBlob, _psMBlob);

	//// 表示用
	//result = bufferGPLSetting->CreateGPStateWrapper(_dev, bufferSetRootSignature, _vsBackbufferBlob, _psBackbufferBlob);

	//// ライトマップ用
	//result = lightMapGPLSetting->CreateGPStateWrapper(_dev, lightMapRootSignature, _lightMapVSBlob, _lightMapPSBlob);

	//// for shrinked bloom creating
	//result = bloomGPLSetting->CreateGPStateWrapper(_dev, bloomRootSignature, _bloomVSBlob, _bloomPSBlob);

	//// AO
	//result = aoGPLSetting->CreateGPStateWrapper(_dev, aoRootSignature, _aoVSBlob, _aoPSBlob);

// 初期化処理5：コマンドリスト生成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator2.Get(), nullptr, IID_PPV_ARGS(_cmdList2.ReleaseAndGetAddressOf()));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator3.Get(), nullptr, IID_PPV_ARGS(_cmdList3.ReleaseAndGetAddressOf()));
	m_batchSubmit[0] = _cmdList.Get();
	m_batchSubmit[1] = _cmdList2.Get();
// 初期化処理6：コマンドリストのクローズ(コマンドリストの実行前には必ずクローズする)

// 初期化処理7：各バッファーを作成して頂点情報を読み込み

	
	//ファイル形式毎のテクスチャロード処理
	textureLoader->LoadTexture();

	// テクスチャアップロード
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
	//// ガウシアンぼかし用ウェイト、バッファー作成、マッピング、ディスクリプタヒープ作成、ビュー作成まで
	//auto weights = Utility::GetGaussianWeight(8, 5.0f);
	//bufferHeapCreator[i]->CreateConstBufferOfGaussian(_dev, weights);
	//mappingExecuter[i]->MappingGaussianWeight(weights);
	////bufferHeapCreator->CreateEffectHeap(_dev);
	////viewCreator->CreateCBV4GaussianView(_dev);

	//// マルチパス用ビュー作成
	peraPolygon->CreatePeraView(_dev);
	//viewCreator[i]->CreateRTV4Multipasses(_dev);
	//viewCreator[i]->CreateSRV4Multipasses(_dev);

// 初期化処理9：フェンスの生成
// 初期化処理10：イベントハンドルの作成
// 初期化処理11：GPUの処理完了待ち

// Imgui独自の初期設定
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

//// DirectXTK独自の初期設定
//	DirectXTKInit();
//	
	for (auto& reManager : resourceManager)
	{
		reManager->ClearReference();
	}

	viewPort = prepareRenderingWindow->GetViewPortPointer();
	rect = prepareRenderingWindow->GetRectPointer();

	// Sky設定
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
	
	// resourceManager[0]のみに格納...
	resourceManager[0]->SetSunResourceAndCreateView(sun->GetRenderResource());
	resourceManager[0]->SetSkyResourceAndCreateView(sky->GetSkyLUTRenderingResource());
	resourceManager[0]->SetImGuiResourceAndCreateView(settingImgui->GetImguiRenderingResource());
	// air(ボリュームライティング)はオブジェクトとキャラクターで利用
	resourceManager[0]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	resourceManager[1]->SetAirResourceAndCreateView(air->GetAirTextureResource());

	return true;
}

void D3DX12Wrapper::Run() {
	MSG msg = {};
	cbv_srv_Size = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//// エフェクトの再生
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

	// 衝突判定準備
	collisionManager = new CollisionManager(_dev, resourceManager);
	//connanDirection = resourceManager[1]->GetMappedMatrix()->world;
	leftSpinMatrix = XMMatrixRotationY(-turnSpeed);
	XMVECTOR det;
	rightSpinMatrix = XMMatrixInverse(&det, leftSpinMatrix);//XMMatrixRotationY(turnSpeed);
	//box2 = collisionManager->GetBoundingSpherePointer();

	//★eigen test
	Matrix3d leftSpinEigen;
	Vector3d axis;
	axis << 0, 1, 0;  //y軸を指定
	leftSpinEigen = AngleAxisd(/*M_PI*/PI*0.006f, axis);  //Z軸周りに90度反時計回りに回転
	leftSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	leftSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	leftSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	leftSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	leftSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	leftSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	leftSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	leftSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	leftSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);

	leftSpinEigen = AngleAxisd(-/*M_PI*/PI*0.006f, axis);  //Z軸周りに90度反時計回りに回転
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
	// メインループに入る前に精度を取得しておく
	if (QueryPerformanceFrequency(&timeFreq) == FALSE) { // この関数で0(FALSE)が帰る時は未対応
		return;
	}
	// 1度取得しておく(初回計算用)
	QueryPerformanceCounter(&timeStart);
	modelPathSize = modelPath.size();
	DWORD sleepTime;

	//std::vector<std::pair<std::string, VertexInfo>> indiceContainer;
	for (int fbxIndex = 0; fbxIndex < modelPathSize; ++fbxIndex) // ★ムーブセマンティクスによりポインタ所有権の移行を試す→Optimized C++ P215 空vectorとresourceManagerのswapでメモリ開放試す
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

	// 影の描画でも利用する
	shadow->SetVertexAndIndexInfo(vbViews, ibViews, itIndiceFirsts, indiceContainer);

	HANDLE event; // fnece用イベント
	
	fBXPipeline = gPLSetting->GetPipelineState().Get();
	fBXRootsignature = setRootSignature->GetRootSignature().Get();

	// DrawBackBufferで利用する
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
	shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get()); // 以降は解像度に変更がある場合のみ描画する
	sun->ChangeSceneMatrix(XMMatrixIdentity());
	while (true)
	{		
		// 今の時間を取得
		QueryPerformanceCounter(&timeEnd);
		// (今の時間 - 前フレームの時間) / 周波数 = 経過時間(秒単位)
		frameTime = static_cast<float>(timeEnd.QuadPart - timeStart.QuadPart) / static_cast<float>(timeFreq.QuadPart);

		if (frameTime < MIN_FREAM_TIME) { // 時間に余裕がある場合
		// ミリ秒に変換
			sleepTime = static_cast<DWORD>((MIN_FREAM_TIME - frameTime) * 1000);

			timeBeginPeriod(1); // 分解能を上げる(Sleep精度向上)
			Sleep(sleepTime);
			timeEndPeriod(1);   // 戻す

			continue;
		}

		timeStart = timeEnd; // 入れ替え

		// ★処理を無くすとかなり軽くなる
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//アプリ終了時にmessageがWM_QUITになる
		if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
		{
			printf("%s", "quit");
			break;
		}

		//auto k = _swapChain->GetCurrentBackBufferIndex();
		settingImgui->DrawImGUI(_dev, _cmdList);

		// 太陽の位置を更新
		sunDir = sun->CalculateDirectionFromDegrees(settingImgui->GetSunAngleX(), settingImgui->GetSunAngleY());
		sun->CalculateViewMatrix();
		shadow->SetVPMatrix(sun->GetShadowViewMatrix(), sun->GetProjMatrix()); // sun->GetViewMatrix()
		skyLUTBuffer.sunDirection.x = sunDir.x;
		skyLUTBuffer.sunDirection.y = sunDir.y;
		skyLUTBuffer.sunDirection.z = sunDir.z;
		skyLUT->SetSkyLUTBuffer(skyLUTBuffer);

		air->SetFrustum(camera->GetDummyFrustum());
		air->SetSceneInfo(sun->GetShadowViewMatrix(), sun->GetProjMatrix(), camera->GetDummyCameraPos(), sun->GetDirection()); // sun->GetViewMatrix()

		// Shadow Factorの解像度変更はプログラムがクラッシュするため一時封印
		// ShadowFactorの解像度に変更がある場合の処理
		//if (settingImgui->GetIsShadowFactorResolutionChanged())
		//{
		//	shadowFactor->ChangeResolution(settingImgui->GetShadowFactorResX(), settingImgui->GetShadowFactorResY());
		//	skyLUT->SetShadowFactorResource(shadowFactor->GetShadowFactorTextureResource().Get());
		//	shadowFactor->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get());
		//	skyLUT->SetBarrierSWTrue();
		//}

		// SkyLUTの解像度に変更がある場合の処理
		if (settingImgui->GetIsSkyLUTResolutionChanged())
		{
			skyLUT->ChangeSkyLUTResolution(settingImgui->GetSkyLUTResX(), settingImgui->GetSkyLUTResY());
			sky->ChangeSkyLUTResourceAndView(skyLUT->GetSkyLUTRenderingResource().Get());
		}

		// Skyの解像度に変更がある場合の処理
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
		WaitForMultipleObjects(threadNum, m_workerFinishedRenderFrame, TRUE, INFINITE); // DrawBackBufferにおけるドローコール直前に置いてもfpsは改善せず...
			// SetEvent(m_workerBeginRenderFrame[1]); // Tell each worker to start drawing.
		//WaitForSingleObject(m_workerFinishedRenderFrame[bbIdx], INFINITE);

		
		// airのコピー用リソース状態をUAVに戻す
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
		
		//コマンドキューの実行
		ID3D12CommandList* cmdLists[] = { _cmdList.Get(), _cmdList2.Get(), _cmdList3.Get() };
		_cmdQueue->ExecuteCommandLists(3, cmdLists);

		//ID3D12CommandList* cmdLists2[] = { _cmdList2.Get() };
		//_cmdQueue->ExecuteCommandLists(1, cmdLists2);

		//ID3D12FenceのSignalはCPU側のフェンスで即時実行
		//ID3D12CommandQueueのSignalはGPU側のフェンスで
		//コマンドキューに対する他のすべての操作が完了した後にフェンス更新
		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

		while (_fence->GetCompletedValue() != _fenceVal)
		{
			
			event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//イベント発生待ち
			WaitForSingleObject(event, INFINITE);
			//イベントハンドルを閉じる
			CloseHandle(event);
		}

		_cmdAllocator->Reset();//コマンド アロケーターに関連付けられているメモリを再利用する
		_cmdList->Reset(_cmdAllocator.Get(), nullptr);

		_cmdAllocator2->Reset();//コマンド アロケーターに関連付けられているメモリを再利用する
		_cmdList2->Reset(_cmdAllocator2.Get(), nullptr);

		_cmdAllocator3->Reset();
		_cmdList3->Reset(_cmdAllocator3.Get(), nullptr);
		//_cmdList->Reset(_cmdAllocator.Get(), nullptr);//コマンドリストを、新しいコマンドリストが作成されたかのように初期状態にリセット
		//_cmdAllocator2->Reset();
		
	
		//// update by imgui
		//SetFov();

		//resourceManager[1]->GetMappedMatrix()->world *= XMMatrixRotationY(0.005f);
		//resourceManager->GetMappedMatrix()->world *= XMMatrixTranslation(0,0,0.03f);

		//フリップしてレンダリングされたイメージをユーザーに表示
		_swapChain->Present(1, 0);	
		bbIdx = _swapChain->GetCurrentBackBufferIndex();//現在のバックバッファをインデックスにて取得
		// ★★★交互にワーカースレッドに処理をさせるのではなく、一つ先のフレームも並列で処理させるようにしたい。

		

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
		static unsigned int WINAPI thunk(LPVOID lpParameter) // LPVOID : 型指定のない、32ビットポインタ
		{
			ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
			instance->threadWorkTest(parameter->threadIndex);

			return 0;
		}
	};

	for (int i = 0; i < threadNum; i++)
	{
		// 以下三つはCreateEventWによるイベント作成 https://learn.microsoft.com/ja-jp/windows/win32/api/synchapi/nf-synchapi-createeventw
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

		// スレッド作成
		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr, // //SECURITY_ATTRIBUTES 構造体へのポインタ またはNULL
			0, //スタックサイズ。0でもよい
			threadwrapper::thunk, // スレッド関数のアドレス
			reinterpret_cast<LPVOID>(&m_threadParameters[i]), // //スレッド関数に渡す引数、またはNULL このケースでは事前に代入したスレッド番号を格納したm_threadHandles[i]のアドレス。LPVOID lpParameterとして扱われる。
			0, // //0(すぐ実行) またはCREATE_SUSPENDED(一時停止)
			nullptr)); // //スレッド識別子。NULLでも可。

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

		//リソースバリアの準備。ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧは..._COMMONを初期状態とする決まり。これはcolor
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
		//リソースバリア：リソースへの複数のアクセスを同期する必要があることをドライバーに通知
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		
		// モデル描画
		localCmdList->RSSetViewports(1, viewPort);
		localCmdList->RSSetScissorRects(1, rect);

		// resourceManager[0]のrtv,dsvに集約している。手法としてはイマイチか...
		auto dsvhFBX = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
		dsvhFBX.ptr += num * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);;
		auto handleFBX = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
		auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handleFBX.ptr += num * inc;

		localCmdList->OMSetRenderTargets(1, &handleFBX, false, &dsvhFBX);
		localCmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

		//画面クリア
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

			// キー入力処理。当たり判定処理も含める。
			if (input->CheckKey(DIK_W)) inputW = true;
			if (input->CheckKey(DIK_LEFT)) inputLeft = true;
			if (input->CheckKey(DIK_RIGHT)) inputRight = true;
			if (input->CheckKey(DIK_UP)) inputUp = true;

			if (resourceManager[fbxIndex]->GetIsAnimationModel())
			{
				// start character with idle animation
				resourceManager[fbxIndex]->MotionUpdate(idleMotionDataNameAndMaxFrame.first, idleMotionDataNameAndMaxFrame.second);

				// ★Switch化できないか？
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

			// ★Switch化できないか？
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
				// 当たり判定処理
				collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, num);
				camera->MoveCamera(forwardSpeed, connanDirection);
				shadow->SetMoveMatrix(forwardSpeed, connanDirection);
			}

			//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
			localCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/);

			//頂点バッファーのCPU記述子ハンドルを設定
			localCmdList->IASetVertexBuffers(0, 1, vbViews[fbxIndex]);

			//★インデックスバッファーのビューを設定
			localCmdList->IASetIndexBuffer(ibViews[fbxIndex]);

			//ディスクリプタヒープ設定およびディスクリプタヒープとルートパラメータの関連付け	
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
			tHandle.ptr += cbv_srv_Size * indiceContainerSize + cbv_srv_Size * textureIndexes[fbxIndex][0] * num; // ★★スレッド1のみ更にスレッド0が処理する最初のテクスチャ数分　+ cbv_srv_Size * textureIndexes[0].second
			int texStartIndex = 3; // テクスチャを格納するディスクリプタテーブル番号の開始位置
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
					// ★パスは既に転送時に使用済。マテリアル名もcharで1byteに書き換えて比較すればいいのでは？
					while (itMaterialAndTextureName->first == itPhonsInfo->first) // TODO:secondを使っていない→容量の無駄遣いなので消す
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

				indiceSize = itIndiceFirst->second.indices.size(); // ★サイズのみのarrayを用意してみる
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
					tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [1]スレッドで処理する次の該当モデルインデックスのテクスチャ数
				//}
				//else if (num == 1 && (i + 1) <= textureIndexes[fbxIndex].size())
				//{
				//	auto itNext = itIndiceFirst - 1;
				//	ofst += itNext->second.indices.size();
				//	tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [0]スレッドで処理する次の該当モデルインデックスのテクスチャ数
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

		//// マテリアルのディスクリプタヒープをルートシグネチャのテーブルにバインドしていく
		//// CBV:1つ(matrix)、SRV:4つ(colortex, graytex, spa, sph)が対象。SetRootSignature.cpp参照。
		//auto materialHandle = bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
		//auto inc = buffSize;
		//auto materialHInc = inc * 5; // 行列cbv + (material cbv+テクスチャsrv+sph srv+spa srv+toon srv)
		//materialHandle.ptr += inc; // この処理の直前に行列用CBVをｺﾏﾝﾄﾞﾘｽﾄにセットしたため
		//unsigned int idxOffset = 0;

		//// (たぶん)DrawIndexedInstancedによる描画の前にSRVからのテクスチャ取得を終えていないとデータがシェーダーに通らない
		//// なお、このパスでのデプスも描画と同時に渡しているが参照出来ないのは、リソース状態がdepth_writeのままだからと思われる
		//localCmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // デプスマップ格納
		//materialHandle.ptr += inc;
		//localCmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ライトマップ格納
		//materialHandle.ptr += inc;

		//for (auto m : pmdMaterialInfo[i]->materials)
		//{
		//	localCmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
		//	//インデックス付きインスタンス化されたプリミティブを描画
		//	localCmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:通常、1:影

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

	// モデル描画
	//_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());
	//_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
	//_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
	//_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());


	//auto dsvh = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	//D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();

	//_cmdList->OMSetRenderTargets(1, &handle, false, &dsvh);
	//_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

	////画面クリア
	//float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	_cmdList->SetGraphicsRootSignature(collisionRootSignature->GetRootSignature().Get());
	//_cmdList->SetDescriptorHeaps(1, resourceManager[0]->GetSRVHeap().GetAddressOf());

	_cmdList->SetPipelineState(colliderGraphicsPipelineSetting->GetPipelineState().Get());

	//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP/*D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP*/);

	if (modelNum == 0)
	{
		//頂点バッファーのCPU記述子ハンドルを設定
		for (int i = 0; i < collisionManager->GetOBBNum(); ++i)
		{
			_cmdList->IASetVertexBuffers(0, 1, collisionManager->GetBoxVBVs(i));
			// インデックスバッファーのビューを設定
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

	////ディスクリプタヒープ設定およびディスクリプタヒープとルートパラメータの関連付け	
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
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア
//	//画面クリア
//	//_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	_cmdList->IASetVertexBuffers(0, 1, viewCreator[modelNum]->GetVbView());
//
//	_cmdList->IASetIndexBuffer(viewCreator[modelNum]->GetIbView());
//
//	_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[modelNum]->GetCBVSRVHeap().GetAddressOf());
//	_cmdList->SetGraphicsRootDescriptorTable
//	(
//		0, // バインドのスロット番号
//		bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//	);
//
//	auto materialHandle2 = bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//	auto inc2 = buffSize;
//	auto materialHInc2 = inc2 * 5; // 行列cbv + (material cbv+テクスチャsrv+sph srv+spa srv+toon srv)
//	materialHandle2.ptr += inc2; // この処理の直前に行列用CBVをｺﾏﾝﾄﾞﾘｽﾄにセットしたため
//	unsigned int idxOffset2 = 0;
//
//	for (auto m : pmdMaterialInfo[modelNum]->materials)
//	{
//		_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle2);
//		//インデックス付きインスタンス化されたプリミティブを描画
//		_cmdList->DrawIndexedInstanced(m.indiceNum, 1, idxOffset2, 0, 0); // instanceid 0:通常、1:影
//
//		materialHandle2.ptr += materialHInc2;
//		idxOffset2 += m.indiceNum;
//	}
//
//	//_cmdList->DrawIndexedInstanced(pmdMaterialInfo[modelNum]->vertNum, 1, 0, 0, 0);
//
//	// ライトマップ状態をﾚﾝﾀﾞﾘﾝｸﾞﾀｰｹﾞｯﾄに変更する
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
//	//// マルチパス1パス目
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
//	// ﾏﾙﾁﾊﾟｽﾘｿｰｽﾊﾞﾘｱ元に戻す
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
//	//リソースバリアの準備。ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧは..._COMMONを初期状態とする決まり。これはcolor
//	D3D12_RESOURCE_BARRIER BarrierDesc = {};
//	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	BarrierDesc.Transition.pResource = bufferHeapCreator[modelNum]->GetMultipassBuff2().Get();
//	BarrierDesc.Transition.Subresource = 0;
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	//リソースバリア：リソースへの複数のアクセスを同期する必要があることをドライバーに通知
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
//	// モデル描画
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
//	// レンダーターゲットと深度ステンシル(両方シェーダーが認識出来ないビュー)はCPU記述子ハンドルを設定してパイプラインに直バインド
//	// なのでこの二種類のビューはマッピングしなかった
//	//_cmdList->OMSetRenderTargets(2, rtvs/*&handle*/, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア
//
//	//画面クリア
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
//	//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// 描画されている複数のモデルを描画していく
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		//頂点バッファーのCPU記述子ハンドルを設定
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[i]->GetVbView());
//
//		//インデックスバッファーのビューを設定
//		_cmdList->IASetIndexBuffer(viewCreator[i]->GetIbView());
//
//		//ディスクリプタヒープ設定および
//		//ディスクリプタヒープとルートパラメータの関連付け
//		//ここでルートシグネチャのテーブルとディスクリプタが関連付く
//		_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[i]->GetCBVSRVHeap().GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable
//		(
//			0, // バインドのスロット番号
//			bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		);
//
//		//////テキストのように同時に二つの同タイプDHをセットすると、グラボによっては挙動が変化する。
//		////// 二つ目のセットによりNS300/Hではモデルが表示されなくなった。
//		//////_cmdList->SetDescriptorHeaps(1, &materialDescHeap);
//		//////_cmdList->SetGraphicsRootDescriptorTable
//		//////(
//		//////	1, // バインドのスロット番号
//		//////	bufferHeapCreator->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		//////);
//
//		// マテリアルのディスクリプタヒープをルートシグネチャのテーブルにバインドしていく
//		// CBV:1つ(matrix)、SRV:4つ(colortex, graytex, spa, sph)が対象。SetRootSignature.cpp参照。
//		auto materialHandle = bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//		auto inc = buffSize;
//		auto materialHInc = inc * 5; // 行列cbv + (material cbv+テクスチャsrv+sph srv+spa srv+toon srv)
//		materialHandle.ptr += inc; // この処理の直前に行列用CBVをｺﾏﾝﾄﾞﾘｽﾄにセットしたため
//		unsigned int idxOffset = 0;
//
//		// (たぶん)DrawIndexedInstancedによる描画の前にSRVからのテクスチャ取得を終えていないとデータがシェーダーに通らない
//		// なお、このパスでのデプスも描画と同時に渡しているが参照出来ないのは、リソース状態がdepth_writeのままだからと思われる
//		_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // デプスマップ格納
//		materialHandle.ptr += inc;
//		_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ライトマップ格納
//		materialHandle.ptr += inc;
//
//		for (auto m : pmdMaterialInfo[i]->materials)
//		{
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
//			//インデックス付きインスタンス化されたプリミティブを描画
//			_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:通常、1:影
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
//		// デプスマップ用バッファの状態を読み込み可能に変える
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
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア
//
//	float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
//	_cmdList->ClearRenderTargetView(baseH, clearColor, 0, nullptr);
//
//	_cmdList->SetGraphicsRootSignature(aoRootSignature->GetRootSignature().Get());
//
//
//	_cmdList->SetPipelineState(aoGPLSetting->GetPipelineState().Get());
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// 描画されている複数のモデルを描画していく
//
//	
//	for (int i = 0; i < strModelNum; ++i)
//	{
//		//頂点バッファーのCPU記述子ハンドルを設定
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[/*modelNum*/i]->GetVbView());
//		//インデックスバッファーのビューを設定
//		_cmdList->IASetIndexBuffer(viewCreator[/*modelNum*/i]->GetIbView());
//
//		//// ﾃﾞﾌﾟｽﾏｯﾌﾟと法線マップ、シーン行列はそれぞれのモデルのものを利用する。
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
//		//インデックス付きインスタンス化されたプリミティブを描画
//		for (auto m : pmdMaterialInfo[/*modelNum*/i]->materials)
//		{
//			//インデックス付きインスタンス化されたプリミティブを描画
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

	bbIdx = _swapChain->GetCurrentBackBufferIndex();//現在のバックバッファをインデックスにて取得
	//auto localCmdList = m_batchSubmit[bbIdx];

	_cmdList3->RSSetViewports(1, viewPort);
	_cmdList3->RSSetScissorRects(1, rect);

	// ﾊﾞｯｸﾊﾞｯﾌｧに描画する
	// ﾊﾞｯｸﾊﾞｯﾌｧ状態をﾚﾝﾀﾞﾘﾝｸﾞﾀｰｹﾞｯﾄに変更する
	barrierDesc4BackBuffer = CD3DX12_RESOURCE_BARRIER::Transition
	(
		_backBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);

	// 各モデルのデブスマップを読み込み可能状態に変更する
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
	//_cmdList3->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

	//float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	_cmdList3->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);

	// 作成したﾃｸｽﾁｬの利用処理
	_cmdList3->SetGraphicsRootSignature(bBRootsignature);


	//_cmdList3->SetDescriptorHeaps(1, skyLUT->GetSkyLUTRenderingHeap().GetAddressOf());
	//gHandle = skyLUT->GetSkyLUTRenderingHeap()->GetGPUDescriptorHandleForHeapStart();
	//_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle); // sponza全体のレンダリング結果


	// 元
	_cmdList3->SetDescriptorHeaps(1, resourceManager[0]->GetSRVHeap().GetAddressOf());

	gHandle = resourceManager[0]->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle); // sponza全体のレンダリング結果

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(1, gHandle); // connanのレンダリング結果

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(2, gHandle); //  sponza全体のデプスマップ

	gHandle.ptr += buffSize;
	_cmdList3->SetGraphicsRootDescriptorTable(3, gHandle); // connanのデプスマップ

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

	// ﾊﾞｯｸﾊﾞｯﾌｧ状態をﾚﾝﾀﾞﾘﾝｸﾞﾀｰｹﾞｯﾄから元に戻す
	barrierDesc4BackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc4BackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList3->ResourceBarrier(1, &barrierDesc4BackBuffer);

	// 各デブスマップを深度書き込み可能状態に変更する
	barrierDesc4DepthMap.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrierDesc4DepthMap.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	//barrierDesc4DepthMap2.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//barrierDesc4DepthMap2.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);
	//_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap2);
	//for (int i = 0; i < strModelNum; ++i)
	//{
	//	// デプスマップ用バッファの状態を書き込み可能に戻す
	//	auto barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
	//	(
	//		bufferHeapCreator[i]->/*GetDepthMapBuff*/GetDepthBuff().Get(),
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		D3D12_RESOURCE_STATE_DEPTH_WRITE
	//	);

	//	// ライトマップ用バッファの状態を書き込み可能に戻す
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
//	//リソースバリアの準備。ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧは..._COMMONを初期状態とする決まり。これはcolor
//	D3D12_RESOURCE_BARRIER BarrierDesc = {};
//	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	BarrierDesc.Transition.pResource = bufferHeapCreator[modelNum]->GetMultipassBuff2().Get();
//	BarrierDesc.Transition.Subresource = 0;
//	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
//	//リソースバリア：リソースへの複数のアクセスを同期する必要があることをドライバーに通知
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
//	// モデル描画
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
//	// レンダーターゲットと深度ステンシル(両方シェーダーが認識出来ないビュー)はCPU記述子ハンドルを設定してパイプラインに直バインド
//	// なのでこの二種類のビューはマッピングしなかった
//	//_cmdList->OMSetRenderTargets(2, rtvs/*&handle*/, false, &dsvh);
//	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア
//
//	//画面クリア
//	float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
//	_cmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
//	_cmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
//	clearColor[0] = 0;
//	clearColor[1] = 0;
//	clearColor[2] = 0;
//	_cmdList->ClearRenderTargetView(handles[2], clearColor, 0, nullptr);
//
//	//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
//	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// 描画されている複数のモデルを描画していく
//
//		//頂点バッファーのCPU記述子ハンドルを設定
//		_cmdList->IASetVertexBuffers(0, 1, viewCreator[modelNum]->GetVbView());
//
//		//インデックスバッファーのビューを設定
//		_cmdList->IASetIndexBuffer(viewCreator[modelNum]->GetIbView());
//
//		//ディスクリプタヒープ設定および
//		//ディスクリプタヒープとルートパラメータの関連付け
//		//ここでルートシグネチャのテーブルとディスクリプタが関連付く
//		_cmdList->SetDescriptorHeaps(1, bufferHeapCreator[modelNum]->GetCBVSRVHeap().GetAddressOf());
//		_cmdList->SetGraphicsRootDescriptorTable
//		(
//			0, // バインドのスロット番号
//			bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		);
//
//		//////テキストのように同時に二つの同タイプDHをセットすると、グラボによっては挙動が変化する。
//		////// 二つ目のセットによりNS300/Hではモデルが表示されなくなった。
//		//////_cmdList->SetDescriptorHeaps(1, &materialDescHeap);
//		//////_cmdList->SetGraphicsRootDescriptorTable
//		//////(
//		//////	1, // バインドのスロット番号
//		//////	bufferHeapCreator->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart()
//		//////);
//
//		// マテリアルのディスクリプタヒープをルートシグネチャのテーブルにバインドしていく
//		// CBV:1つ(matrix)、SRV:4つ(colortex, graytex, spa, sph)が対象。SetRootSignature.cpp参照。
//		auto materialHandle = bufferHeapCreator[modelNum]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
//		auto inc = buffSize;
//		auto materialHInc = inc * 5; // 行列cbv + (material cbv+テクスチャsrv+sph srv+spa srv+toon srv)
//		materialHandle.ptr += inc; // この処理の直前に行列用CBVをｺﾏﾝﾄﾞﾘｽﾄにセットしたため
//		unsigned int idxOffset = 0;
//
//		// (たぶん)DrawIndexedInstancedによる描画の前にSRVからのテクスチャ取得を終えていないとデータがシェーダーに通らない
//		// なお、このパスでのデプスも描画と同時に渡しているが参照出来ないのは、リソース状態がdepth_writeのままだからと思われる
//		_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // デプスマップ格納
//		materialHandle.ptr += inc;
//		_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ライトマップ格納
//		materialHandle.ptr += inc;
//
//		for (auto m : pmdMaterialInfo[modelNum]->materials)
//		{
//			_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
//			//インデックス付きインスタンス化されたプリミティブを描画
//			_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:通常、1:影
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
//		settingImgui->GetFovValue(), // 画角90°
//		static_cast<float>(prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(prepareRenderingWindow->GetWindowWidth()),
//		1.0, // ニア―クリップ
//		100.0 // ファークリップ
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
//		L"DirectX12の魔導書",
//		XMFLOAT2(102,102),
//		Colors::Black
//	);
//	_spriteFont->DrawString
//	(
//		_spriteBatch,
//		L"DirectX12の魔導書",
//		XMFLOAT2(100, 100),
//		Colors::Yellow
//	);
//	_spriteBatch->End();
//}

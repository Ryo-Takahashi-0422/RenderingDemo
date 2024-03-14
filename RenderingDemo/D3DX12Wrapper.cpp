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

// 初期化処理3：頂点入力レイアウトの作成及び
// 初期化処理4：パイプライン状態オブジェクト(PSO)のDesc記述してオブジェクト作成
	result = gPLSetting->CreateGPStateWrapper(_dev, setRootSignature, _vsBlob, _psBlob);
	
	//// コライダー用
	//result = colliderGraphicsPipelineSetting->CreateGPStateWrapper(_dev, collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);

	// ﾊﾞｯｸﾊﾞｯﾌｧ用
	result = peraGPLSetting->CreateGPStateWrapper(_dev, peraSetRootSignature, _vsMBlob, _psMBlob);

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

	// マルチパス用ビュー作成
	peraPolygon->CreatePeraView(_dev);

// 初期化処理9：フェンスの生成
// 初期化処理10：イベントハンドルの作成
// 初期化処理11：GPUの処理完了待ち

// Imgui独自の初期設定
	settingImgui = new SettingImgui;

	if (FAILED(settingImgui->Init(_dev, prepareRenderingWindow)))
	{
		return false;
	}	

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
	camera->MoveCamera(0, XMMatrixIdentity()); // camera view行列初期化　無ければ太陽の初期位置に影響する
	sky->SetFrustum(camera->GetFrustum());

	sun->SetShadowFactorResource(shadowFactorResource.Get());

	shadow = new Shadow(_dev.Get());
	shadow->Init();
	
	air = new Air(_dev.Get(), _fence.Get(), shadow->GetShadowMapResource(), shadowFactor->GetShadowFactorTextureResource());
	air->SetFrustum(camera->GetFrustum());
	air->SetParticipatingMedia(calculatedParticipatingMedia);

	// vsm blur
	shadowRenderingBlur = new Blur(_dev.Get());
	std::string vs = "BlurVertex.hlsl";
	std::string ps = "BlurPixel.hlsl";
	auto pair = Utility::GetHlslFilepath(vs, ps);
	shadowRenderingBlur->Init(pair);
	shadowRenderingBlur->SetRenderingResourse(shadow->GetShadowRenderingResource());
	
	// resourceManager[0]のみに格納...
	resourceManager[0]->SetSunResourceAndCreateView(sun->GetRenderResource());
	resourceManager[0]->SetSkyResourceAndCreateView(sky->GetSkyLUTRenderingResource());
	resourceManager[0]->SetImGuiResourceAndCreateView(settingImgui->GetImguiRenderingResource());
	// air(ボリュームライティング)はオブジェクトとキャラクターで利用
	resourceManager[0]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	resourceManager[1]->SetAirResourceAndCreateView(air->GetAirTextureResource());
	// VSMをセット
	resourceManager[0]->SetVSMResourceAndCreateView(shadowRenderingBlur->GetBlurResource());
	resourceManager[1]->SetVSMResourceAndCreateView(shadowRenderingBlur->GetBlurResource());
	// airのブラー結果をセット
	//resourceManager[0]->SetVSMResourceAndCreateView(airBlur->GetBlurResource());
	//resourceManager[1]->SetVSMResourceAndCreateView(airBlur->GetBlurResource());
	// ブラーしたシャドウマップをセット
	//resourceManager[0]->SetShadowResourceAndCreateView(/*shadow->GetShadowMapResource()*/comBlur->GetBlurTextureResource());
	//resourceManager[1]->SetShadowResourceAndCreateView(/*shadow->GetShadowMapResource()*/comBlur->GetBlurTextureResource());
	// 平行投影ビューを利用したvsmで影を描画する場合に利用する行列をsunより取得する
	resourceManager[0]->SetProjMatrix(sun->GetProjMatrix());
	resourceManager[1]->SetProjMatrix(sun->GetProjMatrix());

	// 画像統合クラスの処理フロー
	// integrationはthread1,2の統合カラー、法線保有状態になる
	// depthMapIntegrationでthread1,2のデプスマップ統合
	// →comBlurでガウシアンブラー
	// →integrationはthread1,2の統合カラー、法線、デプスマップ保有状態になる
	// 
	
	integration = new Integration(_dev.Get(), resourceManager[0]->GetSRVHeap());
	depthMapIntegration = new DepthMapIntegration(_dev.Get(), resourceManager[0]->GetDepthBuff(), resourceManager[0]->GetDepthBuff2());
	//comBlur = new ComputeBlur(_dev.Get(), depthMapIntegration->GetTextureResource());
	calculateSSAO = new CalculateSSAO(_dev.Get(), integration->GetNormalResourse(), /*comBlur->GetBlurTextureResource()*/depthMapIntegration->GetTextureResource()); // ブラーかけずにSSAO計算してもあまり変わらない...
	ssaoBlur = new Blur(_dev.Get());
	ssaoBlur->Init(pair);
	ssaoBlur->SetRenderingResourse(calculateSSAO->GetTextureResource());
	integration->SetResourse1(/*calculateSSAO->GetTextureResource()*/ssaoBlur->GetBlurResource());

	// integrated color blur for 被写界深度
	colorIntegraredBlur = new Blur(_dev.Get());
	colorIntegraredBlur->Init(pair);
	colorIntegraredBlur->SetRenderingResourse(integration->GetColorResourse());
	integration->SetResourse2(colorIntegraredBlur->GetBlurResource());
	integration->SetResourse3(depthMapIntegration->GetTextureResource());
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
	oBBManager = new OBBManager(_dev, resourceManager);
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
		shadow->SetSunPos(sun->GetFixedDirection());

		// VSMを利用する場合は平行投影ビュー行列を利用するため以下のsunDir代入処理を用いる
		skyLUTBuffer.sunDirection.x = sunDir.x;
		skyLUTBuffer.sunDirection.y = sunDir.y;
		skyLUTBuffer.sunDirection.z = sunDir.z;

		// 深度マップを使う場合は透視投影行列によりビュー行列を作成しているため、太陽位置をカメラ位置に合わせて調整している。これに伴い、skyLUTにおける太陽の位置を合わせて調整する必要がある。
		//skyLUTBuffer.sunDirection = sun->GetPersedSunDirection();
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

		//DrawShrinkTextureForBlur(0, cbv_srv_Size); // draw shrink buffer
		//if (settingImgui->GetSSAOBool())
		//{
		//	DrawAmbientOcclusion(0, cbv_srv_Size); // draw AO	
		//}
		//SetFoVSwitch();
		//SetSSAOSwitch();
		//SetBloomColor();
		
		// カメラはキャラクター移動に追従する。太陽はワールド原点(0,0,0)注視の角度指定*100の位置に固定されているため、太陽描画時はビルボード乗算→カメラ位置(追従位置)へ平行移動→太陽方向へ平行移動とする。
		sun->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		shadow->SetBoneMatrix(resourceManager[1]->GetMappedMatrixPointer()); // シャドウマップでのキャラクターアニメーション処理に利用する
		shadow->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		bool airDraw = settingImgui->GetAirBoxChanged();
		if (airDraw)
		{
			air->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get()); // ★shadowを利用
		}		
		skyLUT->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		sky->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
		shadowRenderingBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect); // ★vsm shadowを利用

		resourceManager[0]->SetSceneInfo(shadow->GetShadowPosMatrix(), shadow->GetShadowPosInvMatrix(), shadow->GetShadowView(), camera->GetDummyCameraPos(), sun->GetDirection());
		resourceManager[1]->SetSceneInfo(shadow->GetShadowPosMatrix(), shadow->GetShadowPosInvMatrix(), shadow->GetShadowView(), camera->GetDummyCameraPos(), sun->GetDirection());
		for (int i = 0; i < threadNum; i++)
		{
			SetEvent(m_workerBeginRenderFrame[i]);			
		}
		WaitForMultipleObjects(threadNum, m_workerFinishedRenderFrame, TRUE, INFINITE); // DrawBackBufferにおけるドローコール直前に置いてもfpsは改善せず...
			// SetEvent(m_workerBeginRenderFrame[1]); // Tell each worker to start drawing.
		//WaitForSingleObject(m_workerFinishedRenderFrame[bbIdx], INFINITE);

		// 画像統合処理→SSAO生成
		integration->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);
		colorIntegraredBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);
		depthMapIntegration->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		//comBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		auto proj = camera->GetProj();
		calculateSSAO->SetDraw(settingImgui->GetSSAOBoxChanged());
		calculateSSAO->SetInvVPMatrix(camera->GetView(), camera->GetInvView(), proj, XMMatrixInverse(nullptr, proj));
		calculateSSAO->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get());
		ssaoBlur->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList3.Get(), _fenceVal, viewPort, rect);

		// airのコピー用リソース状態をUAVに戻す
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
		// シャドウマップを深度書き込み可能な状態に戻す
		auto barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
		(
			shadow->GetShadowMapResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		);
		_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);

		// デプスマップ統合クラス コピー用リソース状態をUAVにする
		auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		(
			depthMapIntegration->GetTextureResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

		AllKeyBoolFalse();
		DrawBackBuffer(cbv_srv_Size); // draw back buffer and DirectXTK

		// calculateSSAOにブラーした統合デプスマップを利用する場合はOnにする
		//// comBlur 利用終了により、コピー用リソース状態をUAVにする
		//barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		//(
		//	comBlur->GetBlurTextureResource().Get(),
		//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		//);
		//_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

		// calculateSSAO 利用終了により、コピー用リソース状態をUAVにする
		barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
		(
			calculateSSAO->GetTextureResource().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);
		_cmdList3->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

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

	delete textureTransporter;

	UnregisterClass(prepareRenderingWindow->GetWNDCCLASSEX().lpszClassName, prepareRenderingWindow->GetWNDCCLASSEX().hInstance);


	delete textureLoader;


	delete settingShaderCompile;
	delete gPLSetting;

	
	//delete prepareRenderingWindow;

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

		// マルチターゲットリソースバリア処理
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

		// スレッド毎に参照先を分ける
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

		//リソースバリア：リソースへの複数のアクセスを同期する必要があることをドライバーに通知
		localCmdList->ResourceBarrier(1, &barrierDescFBX);
		localCmdList->ResourceBarrier(1, &barrierDescNorm);
		
		// モデル描画
		localCmdList->RSSetViewports(1, viewPort);
		localCmdList->RSSetScissorRects(1, rect);

		// resourceManager[0]のrtv,dsvに集約している。手法としてはイマイチか...
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
		localCmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア


		//画面クリア
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


		for (int fbxIndex = 0; fbxIndex < modelPath.size(); ++fbxIndex)
		{
			localCmdList->SetGraphicsRootSignature(fBXRootsignature);
			localCmdList->SetPipelineState(fBXPipeline);

			// アフィン変換の操作は単一スレッドのみ実行する。複数スレッドで処理すると、その分挙動速度が倍増する・太陽がぶれるなど悪影響が出るため。
			if (num == 0)
			{
				// キー入力処理。当たり判定処理も含める。
				if (input->CheckKey(DIK_W)) inputW = true;
				if (input->CheckKey(DIK_LEFT)) inputLeft = true;
				if (input->CheckKey(DIK_RIGHT)) inputRight = true;
				if (input->CheckKey(DIK_UP)) inputUp = true;

				if (resourceManager[fbxIndex]->GetIsAnimationModel())
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

						// 平行移動成分にキャラクターの向きから回転成分を乗算して方向変え。これによる回転移動成分は不要なので、1と0にする。Y軸回転のみ対応している。
						auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, forwardSpeed), connanDirection);
						moveMatrix.r[0].m128_f32[0] = 1;
						moveMatrix.r[0].m128_f32[2] = 0;
						moveMatrix.r[2].m128_f32[0] = 0;
						moveMatrix.r[2].m128_f32[2] = 1;
						worldVec = XMVector4Transform(worldVec, moveMatrix); // 符号注意

						//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0] = worldVec.m128_f32[0];
						//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[1] = worldVec.m128_f32[1];
						//resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2] = worldVec.m128_f32[2];
						charaPos = collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, fbxIndex, worldVec, charaPos);

						//charaPos.x = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[0];
						//charaPos.z = resourceManager[fbxIndex]->GetMappedMatrix()->world.r[3].m128_f32[2];
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						//resourceManager[fbxIndex]->GetMappedMatrix()->charaPos = charaPos;
						shadow->SetMoveMatrix(resourceManager[fbxIndex]->GetMappedMatrix()->world);


						
						
						//// デブスマップを読み込み可能状態に変更する
						//D3D12_RESOURCE_BARRIER barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
						//(
						//    shadow->GetShadowMapResource().Get(),
						//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
						//	D3D12_RESOURCE_STATE_DEPTH_WRITE
						//);
						//localCmdList->ResourceBarrier(1, &barrierDesc4DepthMap);
						////shadow->SetBoneMatrix(resourceManager[fbxIndex]->GetMappedMatrixPointer()); // シャドウマップでのキャラクターアニメーション処理に利用する
						//shadow->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get(), _fenceVal, viewPort, rect);
						//// コピー用リソース状態をSkyLUT.hlslで読み込める状態にする
						//D3D12_RESOURCE_BARRIER barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
						//(
						//	air->GetAirTextureResource().Get(),
						//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
						//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS
						//);
						//localCmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);
						//air->Execution(_cmdQueue.Get(), _cmdAllocator.Get(), _cmdList.Get()); // ★shadowを利用
					}

					// Left Key
					if (inputLeft)
					{
						connanDirection *= leftSpinMatrix;
						resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
						resourceManager[fbxIndex]->GetMappedMatrix()->rotation = connanDirection;
						shadow->SetRotationMatrix(connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						collisionManager->SetRotation(connanDirection);
					}

					// Right Key
					if (inputRight)
					{
						connanDirection *= rightSpinMatrix;
						resourceManager[fbxIndex]->MotionUpdate(walkingMotionDataNameAndMaxFrame.first, walkingMotionDataNameAndMaxFrame.second);
						resourceManager[fbxIndex]->GetMappedMatrix()->rotation = connanDirection;
						shadow->SetRotationMatrix(connanDirection);
						resourceManager[fbxIndex]->GetMappedMatrix()->view = camera->CalculateOribitView(charaPos, connanDirection);
						collisionManager->SetRotation(connanDirection);
					}
				}

				// Left Arrow Key
				if (inputLeft && !resourceManager[fbxIndex]->GetIsAnimationModel())
				{
					resourceManager[fbxIndex]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;

					sun->ChangeSceneMatrix(rightSpinMatrix);
					sky->ChangeSceneMatrix(rightSpinMatrix);
				}

				// Right Arrow Key
				if (inputRight && !resourceManager[fbxIndex]->GetIsAnimationModel())
				{
					resourceManager[fbxIndex]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;

					sun->ChangeSceneMatrix(leftSpinMatrix);
					sky->ChangeSceneMatrix(leftSpinMatrix);
				}

				//// Up Arrow Key
				//if (inputUp && !resourceManager[fbxIndex]->GetIsAnimationModel())
				//{
				//	resourceManager[fbxIndex]->GetMappedMatrix()->view *= angleUpMatrix;
				//	sun->ChangeSceneMatrix(XMMatrixInverse(nullptr, angleUpMatrix));
				//	sky->ChangeSceneMatrix(angleUpMatrix);
				//}

				// W Key
				if (inputW && !resourceManager[fbxIndex]->GetIsAnimationModel())
				{
					// 当たり判定処理
					//collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, num);
					resourceManager[fbxIndex]->GetMappedMatrix()->view = resourceManager[1]->GetMappedMatrix()->view;
				}
			}

			// ★キャラクターの回転行列を転置したものをGPUへ渡す。DirectXではCPUから受け取った行列は転置してGPUへ転送される。(or GPUで転置される? 詳しくは不明) CPU側で利用している回転行列とGPUで利用する回転行列を合わせることでGPU側での法線回転計算が上手くいっている認識だが...
			resourceManager[fbxIndex]->GetMappedMatrix()->charaRot = XMMatrixTranspose(connanDirection);

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
			dHandle.ptr += cbv_srv_Size * 8; // airアドレスまで飛ぶ

			int textureindex = 2;
			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // air (ptr num9)
			++textureindex;
			dHandle.ptr += cbv_srv_Size;

			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // vsm (ptr num10)
			++textureindex;
			dHandle.ptr += cbv_srv_Size;

			localCmdList->SetGraphicsRootDescriptorTable(textureindex, dHandle); // depthmap (ptr num11)
			++textureindex;
			dHandle.ptr += cbv_srv_Size * 3; // シャドウマップおよび法線画像2個分の領域は使わないので飛ばす。マジックアンバー化していてややこしい...
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
			int texStartIndex = textureindex; // テクスチャを格納するディスクリプタテーブル番号の開始位置
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

				indiceSize = itIndiceFirst->second.indices.size(); // ★サイズのみのarrayを用意してみる
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
				tHandle.ptr += cbv_srv_Size * textureIndexes[fbxIndex][i + 1]; // [1]スレッドで処理する次の該当モデルインデックスのテクスチャ数
				
				itMaterialAndTextureName += textureIndexes[fbxIndex][i + 1];
				itMATCnt += textureIndexes[fbxIndex][i + 1];
				textureTableStartIndex = texStartIndex; // init

			}
			

		}

		bool colliderDraw = settingImgui->GetCollisionBoxChanged();
		// コライダー描画 thread1(num=0)でキャラクタースフィア、thread2でOBBを描画する
		if (num == 0)
		{
			auto mappedMatrix = resourceManager[0]->GetMappedMatrix();
			collisionManager->SetMatrix(mappedMatrix->world, mappedMatrix->view, mappedMatrix->proj);
			collisionManager->SetCharaPos(charaPos);
			collisionManager->SetDraw(colliderDraw, localCmdList.Get());
			//collisionManager->Execution(localCmdList.Get());

		}
		else
		{
			auto mappedMatrix = resourceManager[0]->GetMappedMatrix();
			oBBManager->SetMatrix(mappedMatrix->world, mappedMatrix->view, mappedMatrix->proj);
			oBBManager->SetDraw(colliderDraw, localCmdList.Get());
			//oBBManager->Execution(localCmdList.Get());
		}

		// マルチターゲットリソースバリア処理
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

	_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);

	rtvHeapPointer = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHeapPointer.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList3->OMSetRenderTargets(1, &rtvHeapPointer, false, /*&dsvh*/nullptr);

	_cmdList3->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);

	// 作成したﾃｸｽﾁｬの利用処理
	_cmdList3->SetGraphicsRootSignature(bBRootsignature);

	_cmdList3->SetDescriptorHeaps(1, integration->GetSRVHeap().GetAddressOf());

	gHandle = integration->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();

	_cmdList3->SetGraphicsRootDescriptorTable(0, gHandle); // color統合
	gHandle.ptr += buffSize * 2;

	_cmdList3->SetGraphicsRootDescriptorTable(1, gHandle); // imgui
	gHandle.ptr += buffSize;

	_cmdList3->SetGraphicsRootDescriptorTable(2, gHandle); // SSAO
	gHandle.ptr += buffSize;

	_cmdList3->SetGraphicsRootDescriptorTable(3, gHandle); // blured color
	gHandle.ptr += buffSize;

	_cmdList3->SetGraphicsRootDescriptorTable(4, gHandle); // depth

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

	_cmdList3->ResourceBarrier(1, &barrierDesc4DepthMap);
}

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

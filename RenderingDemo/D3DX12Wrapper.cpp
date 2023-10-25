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

	// SetRootSignatureBaseクラスのインスタンス化
	setRootSignature = new SetRootSignature;

	

	// SettingShaderCompileクラスのインスタンス化
	settingShaderCompile = new SettingShaderCompile;

	// VertexInputLayoutクラスのインスタンス化
	vertexInputLayout = new VertexInputLayout;


	// GraphicsPipelineSettingクラスのインスタンス化
	gPLSetting = new GraphicsPipelineSetting(vertexInputLayout);

	// レンダリングウィンドウ設定
	prepareRenderingWindow = new PrepareRenderingWindow;
	prepareRenderingWindow->CreateAppWindow();

	// TextureLoaderクラスのインスタンス化
	textureLoader = new TextureLoader;

	// レンダリングウィンドウ表示
	ShowWindow(prepareRenderingWindow->GetHWND(), SW_SHOW);

	// ビューポートとシザー領域の設定
	prepareRenderingWindow->SetViewportAndRect();

	// キーボード入力管理クラス
	input = new Input(prepareRenderingWindow);

	//// ﾏﾙﾁﾊﾟｽ関連ｸﾗｽ群
	peraLayout = new PeraLayout;
	peraGPLSetting = new PeraGraphicsPipelineSetting(peraLayout/*vertexInputLayout*/); //TODO PeraLayout,VertexInputLayoutｸﾗｽの基底クラスを作ってそれに対応させる
	peraPolygon = new PeraPolygon;
	peraSetRootSignature = new PeraSetRootSignature;
	peraShaderCompile = new PeraShaderCompile;
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
	collisionShaderCompile = new CollisionShaderCompile;

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
		(IDXGISwapChain1**)_swapChain.ReleaseAndGetAddressOf());
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
	
	// connan, zig   zigを先に読み込むと問題ないが、後で読み込むとD3D12_GPU_DESCRIPTOR_HANDLEの読み取りエラーが発生する。
	// connan, battlefield   battlefieldを先に読み込むと問題ないが、後で読み込むとD3D12_GPU_DESCRIPTOR_HANDLEの読み取りエラーが発生する。
	// zig, battlefield   battlefieldを先に読み込むと問題ないが、後で読み込むとD3D12_GPU_DESCRIPTOR_HANDLEの読み取りエラーが発生する。
	// battlefield, battlefieldは〇、NewConnan,NewConnanは〇
	// ★テクスチャをDrawFBXでSetGraphicsRootDescriptorTableでバインドしているが、別のメッシュを描画する場合にSetDescriptorHeapsでそのメッシュのSRVに切り替えるが、
	// そのメッシュに利用しているテクスチャ枚数が前回描画したメッシュのテクスチャ枚数より少ないと、SetGraphicsRootDescriptorTableによるバインドの上書き(されていると仮定)が
	// 行われず前回のバインドメモリが残ってしまい、#554 DescriptorHeap Invalidエラーが発生している模様。テクスチャのバインドをコメントアウトでエラー消失すること、
	// エラーメッセージ(現在バインドされているDescriptorheapが現在設定されているものと異なる的な)から推測した。
	// 現状の回避策は、読み込むモデルをテクスチャの少ない順にすることに限られる。
	

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
	// _vsBlobと_psBlobにｼｪｰﾀﾞｰｺﾝﾊﾟｲﾙ設定を割り当てる。それぞれﾌｧｲﾙﾊﾟｽを保持するが読み込み失敗したらnullptrが返ってくる。
	auto blobs = settingShaderCompile->SetShaderCompile(setRootSignature, _vsBlob, _psBlob);
	if (blobs.first == nullptr or blobs.second == nullptr) return false;
	_vsBlob = blobs.first;
	_psBlob = blobs.second;	
	delete settingShaderCompile;

	// ﾏﾙﾁﾊﾟｽ1枚目用
	auto mBlobs = peraShaderCompile->SetPeraShaderCompile(peraSetRootSignature, _vsMBlob, _psMBlob);
	if (mBlobs.first == nullptr or mBlobs.second == nullptr) return false;
	_vsMBlob = mBlobs.first;
	_psMBlob = mBlobs.second;
	delete peraShaderCompile;

	// コライダー用
	auto colliderBlobs = collisionShaderCompile->SetPeraShaderCompile(collisionRootSignature, _vsCollisionBlob, _psCollisionBlob);
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

// 初期化処理6：コマンドリストのクローズ(コマンドリストの実行前には必ずクローズする)
	//cmdList->Close();

// 初期化処理7：各バッファーを作成して頂点情報を読み込み
	//for (int i = 0; i < strModelNum; ++i)
	//{
	
	////頂点バッファーの作成(リソースと暗黙的なヒープの作成) 
	//result = bufferHeapCreator[i]->CreateBufferOfVertex(_dev);

	////インデックスバッファーを作成(リソースと暗黙的なヒープの作成)
	//result = bufferHeapCreator[i]->CreateBufferOfIndex(_dev);

	////デプスバッファーを作成
	//result = bufferHeapCreator[i]->CreateBufferOfDepthAndLightMap(_dev);

	
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
	//	ID3D12Fence* _fence = nullptr;
	//	UINT64 _fenceVal = 0;
	//	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

// 初期化処理10：イベントハンドルの作成
// 初期化処理11：GPUの処理完了待ち

//// Imgui独自の初期設定
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

//// DirectXTK独自の初期設定
//	DirectXTKInit();
//	
	return true;
}

void D3DX12Wrapper::Run() {
	MSG msg = {};
	auto cbv_srv_Size = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
	auto idn = leftSpinMatrix * rightSpinMatrix;

	//★eigen test
	Matrix3d leftSpinEigen;
	Vector3d axis;
	axis << 0, 1, 0;  //y軸を指定
	leftSpinEigen = AngleAxisd(M_PI*0.03f, axis);  //Z軸周りに90度反時計回りに回転
	leftSpinMatrix.r[0].m128_f32[0] = leftSpinEigen(0, 0);
	leftSpinMatrix.r[0].m128_f32[1] = leftSpinEigen(0, 1);
	leftSpinMatrix.r[0].m128_f32[2] = leftSpinEigen(0, 2);
	leftSpinMatrix.r[1].m128_f32[0] = leftSpinEigen(1, 0);
	leftSpinMatrix.r[1].m128_f32[1] = leftSpinEigen(1, 1);
	leftSpinMatrix.r[1].m128_f32[2] = leftSpinEigen(1, 2);
	leftSpinMatrix.r[2].m128_f32[0] = leftSpinEigen(2, 0);
	leftSpinMatrix.r[2].m128_f32[1] = leftSpinEigen(2, 1);
	leftSpinMatrix.r[2].m128_f32[2] = leftSpinEigen(2, 2);

	leftSpinEigen = AngleAxisd(-M_PI*0.03f, axis);  //Z軸周りに90度反時計回りに回転
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
		//アプリ終了時にmessageがWM_QUITになる
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

		//コマンドリストのクローズ(コマンドリストの実行前には必ずクローズする)
		_cmdList->Close();

		//コマンドキューの実行
		ID3D12CommandList* cmdLists[] = { _cmdList.Get() };
		_cmdQueue->ExecuteCommandLists(1, cmdLists);

		//ID3D12FenceのSignalはCPU側のフェンスで即時実行
		//ID3D12CommandQueueのSignalはGPU側のフェンスで
		//コマンドキューに対する他のすべての操作が完了した後にフェンス更新
		_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

		while (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			//イベント発生待ち
			WaitForSingleObject(event, INFINITE);
			//イベントハンドルを閉じる
			CloseHandle(event);
		}

		_cmdAllocator->Reset();//コマンド アロケーターに関連付けられているメモリを再利用する		
		_cmdList->Reset(_cmdAllocator.Get(), nullptr);//コマンドリストを、新しいコマンドリストが作成されたかのように初期状態にリセット
	
		//// update by imgui
		//SetFov();

		//resourceManager[1]->GetMappedMatrix()->world *= XMMatrixRotationY(0.005f);
		//resourceManager->GetMappedMatrix()->world *= XMMatrixTranslation(0,0,0.03f);

		//フリップしてレンダリングされたイメージをユーザーに表示
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
	//リソースバリアの準備。ｽﾜｯﾌﾟﾁｪｰﾝﾊﾞｯｸﾊﾞｯﾌｧは..._COMMONを初期状態とする決まり。これはcolor
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = resourceManager[0]->GetRenderingBuff().Get();
	BarrierDesc.Transition.Subresource = 0;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//リソースバリア：リソースへの複数のアクセスを同期する必要があることをドライバーに通知
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	// モデル描画
	/*_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());*/
	/*_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());*/
	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer());
	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer());


	auto dsvh = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE handle = resourceManager[0]->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();

	_cmdList->OMSetRenderTargets(1, &handle, false, &dsvh);
	_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

	//画面クリア
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	_cmdList->ClearRenderTargetView(handle, clearColor, 0, nullptr);

	int lastSRVSetNum = 0;
	for (int fbxIndex = 0; fbxIndex < modelPath.size(); ++fbxIndex)
	{
		_cmdList->SetGraphicsRootSignature(setRootSignature->GetRootSignature().Get());
		_cmdList->SetPipelineState(gPLSetting->GetPipelineState().Get());

		// キー入力処理。当たり判定処理も含める。
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
			// 当たり判定処理
			collisionManager->OBBCollisionCheckAndTransration(forwardSpeed, connanDirection, fbxIndex);
		}

		//プリミティブ型に関する情報と、入力アセンブラーステージの入力データを記述するデータ順序をバインド
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST/*D3D_PRIMITIVE_TOPOLOGY_POINTLIST*/);

		//頂点バッファーのCPU記述子ハンドルを設定
		_cmdList->IASetVertexBuffers(0, 1, resourceManager[fbxIndex]->GetVbView());

		//★インデックスバッファーのビューを設定
		_cmdList->IASetIndexBuffer(resourceManager[fbxIndex]->GetIbView());

		//ディスクリプタヒープ設定およびディスクリプタヒープとルートパラメータの関連付け	
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
	//// マテリアルのディスクリプタヒープをルートシグネチャのテーブルにバインドしていく
	//// CBV:1つ(matrix)、SRV:4つ(colortex, graytex, spa, sph)が対象。SetRootSignature.cpp参照。
	//auto materialHandle = bufferHeapCreator[i]->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart();
	//auto inc = buffSize;
	//auto materialHInc = inc * 5; // 行列cbv + (material cbv+テクスチャsrv+sph srv+spa srv+toon srv)
	//materialHandle.ptr += inc; // この処理の直前に行列用CBVをｺﾏﾝﾄﾞﾘｽﾄにセットしたため
	//unsigned int idxOffset = 0;

	//// (たぶん)DrawIndexedInstancedによる描画の前にSRVからのテクスチャ取得を終えていないとデータがシェーダーに通らない
	//// なお、このパスでのデプスも描画と同時に渡しているが参照出来ないのは、リソース状態がdepth_writeのままだからと思われる
	//_cmdList->SetGraphicsRootDescriptorTable(2, materialHandle); // デプスマップ格納
	//materialHandle.ptr += inc;
	//_cmdList->SetGraphicsRootDescriptorTable(3, materialHandle); // ライトマップ格納
	//materialHandle.ptr += inc;

	//for (auto m : pmdMaterialInfo[i]->materials)
	//{
	//	_cmdList->SetGraphicsRootDescriptorTable(1, materialHandle);
	//	//インデックス付きインスタンス化されたプリミティブを描画
	//	_cmdList->DrawIndexedInstanced(m.indiceNum, 2, idxOffset, 0, 0); // instanceid 0:通常、1:影

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

	auto bbIdx = _swapChain->GetCurrentBackBufferIndex();//現在のバックバッファをインデックスにて取得
	
	_cmdList->RSSetViewports(1, prepareRenderingWindow->GetViewPortPointer()); // 実は重要
	_cmdList->RSSetScissorRects(1, prepareRenderingWindow->GetRectPointer()); // 実は重要

	// ﾊﾞｯｸﾊﾞｯﾌｧに描画する
	// ﾊﾞｯｸﾊﾞｯﾌｧ状態をﾚﾝﾀﾞﾘﾝｸﾞﾀｰｹﾞｯﾄに変更する
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
	//_cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	_cmdList->ClearRenderTargetView(rtvHeapPointer, clsClr, 0, nullptr);

	// 作成したﾃｸｽﾁｬの利用処理
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

	// ﾊﾞｯｸﾊﾞｯﾌｧ状態をﾚﾝﾀﾞﾘﾝｸﾞﾀｰｹﾞｯﾄから元に戻す
	barrierDesc4BackBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc4BackBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &barrierDesc4BackBuffer);

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

#include <stdafx.h>
#include <Sun.h>

Sun::Sun(ID3D12Device* dev, Camera* camera)
{
    _dev = dev;
    _camera = camera;
}

void Sun::Init()
{
	CreateSunVertex();
    mappedMatrix = new BillboardMatrix;
    
    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();
    
    RenderingSet();
    DrawResourceSet();
    InitBillboardMatrixReosources();
}

void Sun::CreateSunVertex()
{
	auto div = 2 * PI / vertexCnt;
	auto rad = div;
	XMVECTOR ori = { 0,0,0,1 };
    float sunDiskSize_ = 0.03f;
    for (int i = 0; i < vertexCnt * 2; ++i)
	{
		if (i % 3 == 0)
		{
			vertexes.push_back(ori);
			indices.push_back(i);
			vertexCnt++;
            rad -= div;
			continue;
		}

		XMVECTOR pos = { cos(rad), sin(rad), 0,1 };

		vertexes.push_back(pos * sunDiskSize_);
		indices.push_back(i);

		rad += div;
	}
}

void Sun::CalculateBillbordMatrix()
{
    auto fixedDir = direction;
    //XMStoreFloat3(&fixedDir,XMLoadFloat3(&charaPos));
    fixedDir.y *= -1;
    XMVECTOR zDir = XMLoadFloat3(&fixedDir);
    XMVECTOR xDir, yDir;
    //yDir = XMVector3Cross(zDir, xDir);    

    XMFLOAT3 up = { 0,1,0};
    XMFLOAT3 right = { 1,0,0};
    float rv{};
    const auto&& v1{ DirectX::XMLoadFloat3(&fixedDir) }, && v2{ DirectX::XMLoadFloat3(&right) };
    DirectX::XMStoreFloat(&rv, DirectX::XMVector3Dot(v1, v2));

    if (std::abs(std::abs(rv) - 1) < 0.5f)
        yDir = XMVector3Cross(zDir, XMLoadFloat3(&up));
    else
        yDir = XMVector3Cross(zDir, XMLoadFloat3(&right));
    xDir = XMVector3Cross(yDir, zDir);

	XMMATRIX billBoardMatrix = XMMatrixIdentity();
    //billBoardMatrix = XMMatrixMultiply(sceneMatrix, billBoardMatrix);

	billBoardMatrix.r[0] = xDir;
	billBoardMatrix.r[1] = yDir;
	billBoardMatrix.r[2] = zDir;

    auto cameraPos = _camera->GetDummyCameraPos();
    XMMATRIX cameraPosMatrix = XMMatrixIdentity();
    cameraPosMatrix.r[3].m128_f32[0] = /*cameraPos.x*/0;
    cameraPosMatrix.r[3].m128_f32[1] = /*cameraPos.y*/1.5;
    cameraPosMatrix.r[3].m128_f32[2] = /*cameraPos.z*/0;

    // 太陽の位置合わせ苦肉策。カメラビュー行列は原点固定のため、実際のカメラが原点を離れる=オブジェクトが動く場合に太陽が見え始めた実際のカメラ位置は移動しないため、Dummy位置を取得してカメラ位置の変化と対応させることが出来ない。dummyの変化量に合わせて太陽角度を変更させるしかない。
    //auto cal = _camera->GetDummyCameraPos();
    //cal.x *= 0.012; // 現合値。システムの都合で2024/2/12時点での苦しい対策...
    //auto newdir = fixedDir;
    //newdir.x -= cal.x;
    //float theta = atan2(newdir.y, newdir.x);
    //fixedDir.x = cos(theta);
    //fixedDir.y = sin(theta);
    //fixedDir.y = std::min(std::max(fixedDir.y, 0.0f), 1.0f);
    //XMStoreFloat3(&fixedDir, XMVector3Normalize(XMLoadFloat3(&fixedDir)));

    XMVECTOR invSunDir = { fixedDir.x, fixedDir.y, fixedDir.z , 1 };
    XMMATRIX sunDirMatrix = XMMatrixIdentity();
    sunDirMatrix.r[3].m128_f32[0] = invSunDir.m128_f32[0];
    sunDirMatrix.r[3].m128_f32[1] = invSunDir.m128_f32[1]/* * invSunDir.m128_f32[1]*/; // 2乗するとshadowmapの視点高さと大体合う。カメラの移動に対しても合うが、偶然と思われる。当然skyLUTの輝き中心からは少しずれる。
    sunDirMatrix.r[3].m128_f32[2] = invSunDir.m128_f32[2];

    auto cameraPos4Shadow = cameraPos;
    cameraPos4Shadow.x = 0; // 一旦原点注視で
    cameraPos4Shadow.y = 0;
    cameraPos4Shadow.z = 0;
    //auto target = XMFLOAT3(cameraPos4Shadow.x, cameraPos4Shadow.y, cameraPos4Shadow.z - 2.3);

    shadowViewMatrix = XMMatrixLookAtLH
    (
        XMLoadFloat3(&expFixedDir),
        XMLoadFloat3(&cameraPos4Shadow),
        XMLoadFloat3(&up)
    );

    mappedMatrix->world = XMMatrixIdentity();
    mappedMatrix->view = XMMatrixMultiply(_camera->GetView(), sceneMatrix);
    mappedMatrix->proj = _camera->GetProj();
    mappedMatrix->cameraPos = cameraPosMatrix;
    mappedMatrix->sunDir = sunDirMatrix;
    mappedMatrix->billborad = billBoardMatrix;
    mappedMatrix->sunTheta = direction;
}

XMFLOAT3 Sun::CalculateDirectionFromDegrees(float angleX, float angleY)
{	
	float xRad = XMConvertToRadians(angleX);
	float yRad = XMConvertToRadians(angleY);
	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // ビルボード化のためyを負にする。太陽→カメラへのベクトル
    direction = pos;
	return direction;
}

void Sun::ChangeSceneMatrix(XMMATRIX _world)
{
    sceneMatrix *= _world;
    mappedMatrix->scene = sceneMatrix;
}

void Sun::CalculateViewMatrix()
{
    //ビュー行列の生成・乗算
    XMFLOAT3 target(0, 0, 0);
    XMFLOAT3 up(0, 1, 0);
    auto fixedDir = direction;
    fixedDir.x *=100;
    fixedDir.y *= -100;
    fixedDir.z *= 100;
    expFixedDir = fixedDir;

    sunViewMatrix = XMMatrixLookAtLH
    (
        XMLoadFloat3(&fixedDir),
        XMLoadFloat3(&target),
        XMLoadFloat3(&up)
    );

    //プロジェクション(射影)行列の生成・乗算
    sunProjMatrix = XMMatrixPerspectiveFovLH
    (
        XM_PIDIV2, // 画角90°
        1,
        1.0, // ニア―クリップ
        3000.0 // ファークリップ
    );
}

// ルートシグネチャ設定
HRESULT Sun::CreateRootSignature()
{
    //サンプラー作成
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //ディスクリプタテーブルのスロット設定
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // participatingMediaパラメータ
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // ShadowFactorテクスチャ

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
    rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 2;
    rootSignatureDesc.pParameters = rootParam;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = stSamplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    auto result = D3D12SerializeRootSignature //シリアル化
    (
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        rootSigBlob.ReleaseAndGetAddressOf(),
        errorBlob.GetAddressOf()
    );

    result = _dev->CreateRootSignature
    (
        0,
        rootSigBlob->GetBufferPointer(),
        rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())
    );

    rootSigBlob->Release();

    return result;
}

//  シェーダー設定
HRESULT Sun::ShaderCompile()
{
    std::string vs = "SunVertex.hlsl";
    std::string ps = "SunPixel.hlsl";
    auto pair = Utility::GetHlslFilepath(vs, ps);

    auto result = D3DCompileFromFile
    (
        pair.first,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        _vsBlob.ReleaseAndGetAddressOf()
        , _errorBlob.GetAddressOf()
    );

    result = D3DCompileFromFile
    (
        pair.second,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        _psBlob.ReleaseAndGetAddressOf()
        , _errorBlob.GetAddressOf()
    );

    //エラーチェック
    if (FAILED(result))
    {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            ::OutputDebugStringA("ファイルが見つかりません");
            _vsBlob = nullptr;
            _psBlob = nullptr;
            return result;
        }
        else
        {
            std::string errstr;
            errstr.resize(_errorBlob->GetBufferSize());

            std::copy_n((char*)_errorBlob->GetBufferPointer(),
                _errorBlob->GetBufferSize(),
                errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
    }

    return result;
}

// 
void Sun::SetInputLayout()
{
    // 座標
    inputLayout[0] =
    {
        "POSITION",
        0, // 同じセマンティクスに対するインデックス
        DXGI_FORMAT_R32G32_FLOAT,
        0, // スロットインデックス
        D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0 // 一度に描画するインスタンス数

    };

    // UV
    inputLayout[1] =
    {
        "TEXCOORD",
        0,
        DXGI_FORMAT_R32G32_FLOAT,
        0, // スロットインデックス
        D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0
    };
}

// パイプラインの生成
HRESULT Sun::CreateGraphicPipeline()
{
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetDesc = {};
    renderTargetDesc.BlendEnable = false;//ブレンドを有効にするか無効にするか
    renderTargetDesc.LogicOpEnable = false;//論理操作を有効にするか無効にするか
    renderTargetDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSignature.Get();

    if (_vsBlob != nullptr)
    {
        desc.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
        desc.VS.BytecodeLength = _vsBlob->GetBufferSize();
    }

    if (_psBlob != nullptr)
    {
        desc.PS.pShaderBytecode = _psBlob->GetBufferPointer();
        desc.PS.BytecodeLength = _psBlob->GetBufferSize();
    }

    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.RasterizerState.DepthClipEnable = true;
    desc.BlendState.AlphaToCoverageEnable = false;
    desc.BlendState.IndependentBlendEnable = false;
    desc.BlendState.RenderTarget[0] = renderTargetDesc;
    desc.InputLayout.pInputElementDescs = &inputLayout[0];
    desc.InputLayout.NumElements = _countof(inputLayout);
    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // model
    desc.SampleDesc.Count = 1; //1サンプル/ピクセル
    desc.SampleDesc.Quality = 0;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE/*D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT*/;
    desc.DepthStencilState.DepthEnable = false;
    //desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 深度バッファーに深度値を描き込む
    //desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // ソースデータがコピー先データより小さい場合書き込む
    //desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    return result;
}

// RenderingTarget設定
void Sun::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
}

// RenderingTarget RTV,SRV用ヒープの生成
HRESULT Sun::CreateRenderingHeap()
{
    // RTV用
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV用ディスクリプタヒープ
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
    if (result != S_OK)
        return result;

    // SRV用
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    srvHeapDesc.NumDescriptors = 1; // matrix, shadowfactor tex
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget用リソースの生成
HRESULT Sun::CreateRenderingResource()
{
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    float clsClr[4] = { 0.5,0.5,0.5,1.0 };
    auto depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = width;
    resDesc.Height = height;
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
        IID_PPV_ARGS(renderingResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;
}

// RenderingTarget用RTVの作成
void Sun::CreateRenderingRTV()
{
    auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    _dev->CreateRenderTargetView
    (
        renderingResource.Get(),
        &rtvDesc,
        handle
    );
}

// RenderingTarget用SRVの生成
void Sun::CreateRenderingSRV()
{
    auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        renderingResource.Get(),
        &srvDesc,
        handle
    );
}

// 頂点・インデックス関連設定
void Sun::DrawResourceSet()
{
    CreateDrawResourceResource();
    CreateDrawResourceView();
    MappingDrawResource();
}

// リソースの生成
HRESULT Sun::CreateDrawResourceResource()
{
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertexBuffSize = vertexes.size() * sizeof(XMVECTOR);
    auto vertresDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBuffSize);

    auto result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &vertresDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    auto indiceBuffSize = indices.size() * sizeof(unsigned int);
    auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);
    result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &indicesDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
    );
        
    return result;
}

// viewの生成
void Sun::CreateDrawResourceView()
{
    vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファの仮想アドレス
    vbView.SizeInBytes = vertexes.size() * sizeof(XMVECTOR);//全バイト数
    vbView.StrideInBytes = sizeof(XMVECTOR);//1頂点あたりのバイト数

    ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
    ibView.SizeInBytes = indices.size() * sizeof(unsigned int);
    ibView.Format = DXGI_FORMAT_R32_UINT;
}

HRESULT Sun::MappingDrawResource()
{
    auto result = vertBuff->Map(0, nullptr, (void**)&mappedVertPos);
    if (result != S_OK) return result;
    std::copy(std::begin(vertexes), std::end(vertexes), mappedVertPos);
    vertBuff->Unmap(0, nullptr);

    result = idxBuff->Map(0, nullptr, (void**)&mappedIdx); // mapping
    if (result != S_OK) return result;
    std::copy(std::begin(indices), std::end(indices), mappedIdx);
    idxBuff->Unmap(0, nullptr);

    return result;
}

// billboard設定
void Sun::InitBillboardMatrixReosources()
{
    CreateBillboardMatrixHeap();
    CreateBillboardMatrixResources();
    CreateBillboardMatrixView();
    MappingBillboardMatrix();
}

HRESULT Sun::CreateBillboardMatrixHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV用ディスクリプタヒープ
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 2; // matrix, shadowfactor
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(matrixHeap.ReleaseAndGetAddressOf()));
    return result;
}

HRESULT Sun::CreateBillboardMatrixResources()
{
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(BillboardMatrix) + 0xff) & ~0xff);

    auto result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(matrixResource.ReleaseAndGetAddressOf())
    );

    return result;
}

void Sun::CreateBillboardMatrixView()
{
    auto handle = matrixHeap->GetCPUDescriptorHandleForHeapStart();

    // frustum用view
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = matrixResource->GetGPUVirtualAddress();
    desc.SizeInBytes = matrixResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &desc,
        handle
    );
}

HRESULT Sun::MappingBillboardMatrix()
{
    auto result = matrixResource->Map(0, nullptr, (void**)&mappedMatrix);
    return result;
}

void Sun::SetShadowFactorResource(ID3D12Resource* _shadowFactorRsource)
{
    shadowFactorResource = _shadowFactorRsource;
    auto handle = matrixHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    handle.ptr += inc;

    // SkyLUTBuffer用view
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        shadowFactorResource.Get(),
        &srvDesc,
        handle
    );
}

// 実行
void Sun::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
    CalculateBillbordMatrix();
    auto barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    _cmdList->RSSetViewports(1, _viewPort);
    _cmdList->RSSetScissorRects(1, _rect);

    //auto dsvhFBX = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    //dsvhFBX.ptr += num * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _cmdList->OMSetRenderTargets(1, &heapHandle, false, nullptr);
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    _cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);
    //_cmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, matrixHeap.GetAddressOf());

    auto handle = matrixHeap->GetGPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(0, handle); // matrix
    handle.ptr += inc;
    _cmdList->SetGraphicsRootDescriptorTable(1, handle); // shadowfactor

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _cmdList->IASetVertexBuffers(0, 1, &vbView);
    _cmdList->IASetIndexBuffer(&ibView);
    _cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);
}
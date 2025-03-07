#include <stdafx.h>
#include <OcclusionCullingManager.h>

OcclusionCullingManager::OcclusionCullingManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, int _width, int _height)
{
    _dev = dev;
    _fbxInfoManager = fbxInfoManager;
    width = _width;
    height = _height;

    Init();
}

OcclusionCullingManager::~OcclusionCullingManager()
{
    matrixResource->Unmap(0, nullptr);
    mappedMatrix = nullptr;

    mappedVertPos = nullptr;
    mappedIdx = nullptr;
}

void OcclusionCullingManager::Init()
{
    ResourceInit();

    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();

    RenderingSet();
    InitWVPMatrixReosources();
}

HRESULT OcclusionCullingManager::ResourceInit()
{
    // 各メッシュの回転・平行移動行列を作る
    auto localPosAndRotOfOCC = _fbxInfoManager->GetLocalPosAndRotOfOCC();
    localMatrix.resize(localPosAndRotOfOCC.size());
    auto itlPosRot = localPosAndRotOfOCC.begin();
    for (int i = 0; i < localMatrix.size(); ++i)
    {
        // 回転成分の抽出・反映 ※Y軸(UP)回転のみ対応
        auto localRotation = itlPosRot->second.second;
        localRotation.x = XMConvertToRadians(localRotation.x);
        localRotation.y = XMConvertToRadians(localRotation.y);
        localRotation.z = XMConvertToRadians(localRotation.z);
        //XMMATRIX localRotaionXMatrix = XMMatrixRotationX(localRotation.x);
        XMMATRIX localRotaionYMatrix = XMMatrixRotationY(localRotation.y); // blenderと符号逆
        //XMMATRIX localRotaionZMatrix = XMMatrixRotationZ(localRotation.z);
        localMatrix[i] = localRotaionYMatrix/*XMMatrixIdentity()*/;

        // 平行移動成分の抽出・反映
        localMatrix[i].r[3].m128_f32[0] = itlPosRot->second.first.x;
        localMatrix[i].r[3].m128_f32[1] = itlPosRot->second.first.y;
        localMatrix[i].r[3].m128_f32[2] = itlPosRot->second.first.z;

        ++itlPosRot;
    }

    // メッシュの頂点情報を取得する		
    vertMap = _fbxInfoManager->GetIndiceAndVertexInfoOfOCC();

    auto itFirst = vertMap.begin();
    // 各メッシュの頂点座標にローカル回転平行移動行列を乗算してワールド空間へ配置するための座標に変換
    for (int i = 0; i < vertMap.size(); ++i)
    {
        for (int j = 0; j < itFirst->second.vertices.size(); ++j)
        {
            XMFLOAT3 vertCopy = itFirst->second.vertices[j].pos;
            XMVECTOR vertexPos = XMLoadFloat3(&vertCopy);
            vertexPos = XMVector3Transform(vertexPos, localMatrix[i]);
            vertCopy.x = vertexPos.m128_f32[0];
            vertCopy.y = vertexPos.m128_f32[1];
            vertCopy.z = vertexPos.m128_f32[2];
            itFirst->second.vertices[j].pos = vertCopy;
        }
        ++itFirst;
    }

    itFirst = vertMap.begin();
    // create pos container
    for (int i = 0; i < vertMap.size(); ++i)
    {
        for (int j = 0; j < itFirst->second.vertices.size(); ++j)
        {
            verticesPosContainer.push_back(itFirst->second.vertices[j]);
        }
        ++itFirst;
    }

    // デバッグ用。コライダーのみ読み込んだ際の回避処理。
    if (verticesPosContainer.size() == 0)
    {
        FBXVertex dummy = { {0,0,0},{0,0,0},{0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} };
        verticesPosContainer.push_back(dummy);
    }

    auto vertexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto vertexBuffSize = verticesPosContainer.size() * sizeof(FBXVertex);
    auto vertresDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBuffSize);

    auto result = _dev->CreateCommittedResource
    (
        &vertexHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &vertresDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(vertBuff.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    auto vertexTotalNum = verticesPosContainer.size();
    result = vertBuff->Map(0, nullptr, (void**)&mappedVertPos); // mapping
    std::copy(std::begin(verticesPosContainer), std::end(verticesPosContainer), mappedVertPos);
    vertBuff->Unmap(0, nullptr);

    vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファの仮想アドレス
    vbView.SizeInBytes = vertexBuffSize;//全バイト数
    vbView.StrideInBytes = sizeof(FBXVertex);//1頂点あたりのバイト数

    // index Resource
    itFirst = vertMap.begin();
    // create pos container
    for (int i = 0; i < vertMap.size(); ++i)
    {
        for (int j = 0; j < itFirst->second.indices.size(); ++j)
        {
            indexContainer.push_back(itFirst->second.indices[j]);
        }
        ++itFirst;
    }

    auto indexNum = indexContainer.size();
    auto indiceBuffSize = indexNum * sizeof(unsigned int);
    auto indicesDesc = CD3DX12_RESOURCE_DESC::Buffer(indiceBuffSize);
    result = _dev->CreateCommittedResource
    (
        &vertexHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &indicesDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(idxBuff.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    result = idxBuff->Map(0, nullptr, (void**)&mappedIdx); // mapping
    std::copy(std::begin(indexContainer), std::end(indexContainer), mappedIdx);
    idxBuff->Unmap(0, nullptr);

    ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
    ibView.SizeInBytes = indiceBuffSize;
    ibView.Format = DXGI_FORMAT_R32_UINT;

    _fbxInfoManager = nullptr;

    return result;
}

// ルートシグネチャ設定
HRESULT OcclusionCullingManager::CreateRootSignature()
{
    //サンプラー作成
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //ディスクリプタテーブルのスロット設定
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // WVP

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
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

    //rootSigBlob->Release();

    return result;
}

//  シェーダー設定
HRESULT OcclusionCullingManager::ShaderCompile()
{
    std::string vs = "OcclusionCullingVertex.hlsl";
    std::string ps = "OcclusionCullingPixel.hlsl";
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
void OcclusionCullingManager::SetInputLayout()
{
    // 座標
    inputLayout =
    {
        {
            "POSITION",
            0, // 同じセマンティクスに対するインデックス
            DXGI_FORMAT_R32G32B32_FLOAT,
            0, // スロットインデックス
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0 // 一度に描画するインスタンス数

        }
    };
}

// パイプラインの生成
HRESULT OcclusionCullingManager::CreateGraphicPipeline()
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
    desc.InputLayout.NumElements = inputLayout.size();
    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // model
    desc.SampleDesc.Count = 1; //1サンプル/ピクセル
    desc.SampleDesc.Quality = 0;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.DepthStencilState.DepthEnable = true;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 深度バッファーに深度値を描き込む
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // ソースデータがコピー先データより小さい場合書き込む
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget設定
void OcclusionCullingManager::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
    CreateRenderingDSV();
}

// RenderingTarget RTV,SRV,DSV用ヒープの生成
HRESULT OcclusionCullingManager::CreateRenderingHeap()
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
    srvHeapDesc.NumDescriptors = 1; // matrix
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    // DSV用
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1; // 通常の深度マップ
    result = _dev->CreateDescriptorHeap
    (
        &dsvHeapDesc,
        IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf())
    );

    return result;
}

// RenderingTarget用リソースの生成
HRESULT OcclusionCullingManager::CreateRenderingResource()
{
    // レンダリング用
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

    // 深度マップ用
    auto depthHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC depthResDesc = {};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResDesc.Width = width;
    depthResDesc.Height = height;
    depthResDesc.DepthOrArraySize = 1;
    depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS; // 深度値書き込み用
    depthResDesc.SampleDesc.Count = 1; // 1pixce/1つのサンプル
    depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE dValue = {};
    dValue.DepthStencil.Depth = 1.0f;
    dValue.Format = DXGI_FORMAT_D32_FLOAT;

    result = _dev->CreateCommittedResource
    (
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthResDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &dValue,
        IID_PPV_ARGS(depthBuff.ReleaseAndGetAddressOf())
    );

    return result;
}

// RenderingTarget用RTVの作成
void OcclusionCullingManager::CreateRenderingRTV()
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
void OcclusionCullingManager::CreateRenderingSRV()
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

// RenderingTarget用DSVの作成
void OcclusionCullingManager::CreateRenderingDSV()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    auto dsvHandle = dsvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
    // sponza深度マップ用
    _dev->CreateDepthStencilView
    (
        depthBuff.Get(),
        &dsvDesc,
        dsvHandle
    );
}

// WVP設定
void OcclusionCullingManager::InitWVPMatrixReosources()
{
    CreateWVPMatrixHeap();
    CreateWVPMatrixResources();
    CreateWVPMatrixView();
    MappingWVPMatrix();
}


HRESULT OcclusionCullingManager::CreateWVPMatrixHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV用ディスクリプタヒープ
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1; // matrix
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(matrixHeap.ReleaseAndGetAddressOf()));
    return result;
}

HRESULT OcclusionCullingManager::CreateWVPMatrixResources()
{
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(WVPMatrix) + 0xff) & ~0xff);

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

void OcclusionCullingManager::CreateWVPMatrixView()
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

HRESULT OcclusionCullingManager::MappingWVPMatrix()
{
    auto result = matrixResource->Map(0, nullptr, (void**)&mappedMatrix);
    return result;
}

void OcclusionCullingManager::SetVPMatrix(XMMATRIX _view, XMMATRIX _proj)
{
    mappedMatrix->world = XMMatrixIdentity();
    mappedMatrix->view = _view;
    mappedMatrix->proj = _proj;
}

void OcclusionCullingManager::SetSunPos(XMFLOAT3 _sunPos)
{
    mappedMatrix->lightPos = _sunPos;
}

void OcclusionCullingManager::SetMoveMatrix(XMMATRIX charaWorldMatrix)
{
    m_moveMatrix = charaWorldMatrix;
}

//void OcclusionCullingManager::UpdateWorldMatrix()
//{
//    mappedMatrix->world = XMMatrixMultiply(mappedMatrix->world, m_moveMatrix);
//}

// 実行
void OcclusionCullingManager::Execution(ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
    //UpdateWorldMatrix();// キャラクターの移動用行列を更新する

    auto barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    D3D12_VIEWPORT viewport = *_viewPort;
    D3D12_RECT rect = *_rect;
    viewport.Width = width;
    viewport.Height = height;
    rect.right = width;
    rect.bottom = height;

    _cmdList->RSSetViewports(1, &viewport);
    _cmdList->RSSetScissorRects(1, &rect);

    auto dsvh = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _cmdList->OMSetRenderTargets(1, &heapHandle, false, &dsvh);
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    _cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);
    _cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 深度バッファーをクリア

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, matrixHeap.GetAddressOf());

    auto handle = matrixHeap->GetGPUDescriptorHandleForHeapStart();
    //auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(0, handle); // matrix

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    int ofst = 0;
    int indiceSize = 0;
    auto itIndiceFirst = vertMap.begin();
    int indiceContainerSize = vertMap.size();
    _cmdList->IASetVertexBuffers(0, 1, &vbView);
    _cmdList->IASetIndexBuffer(&ibView);

    for (int j = 0; j < indiceContainerSize; ++j)
    {
        indiceSize = itIndiceFirst->second.indices.size(); // ★サイズのみのarrayを用意してみる
        _cmdList->DrawIndexedInstanced(indiceSize, 1, ofst, 0, 0);
        ofst += indiceSize;
        ++itIndiceFirst;
    }
    

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    // デブスマップを読み込み可能状態に変更する
    //D3D12_RESOURCE_BARRIER barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
    //(
    //    depthBuff.Get(),
    //    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    //    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    //);
    //_cmdList->ResourceBarrier(1, &barrierDesc4DepthMap);
}
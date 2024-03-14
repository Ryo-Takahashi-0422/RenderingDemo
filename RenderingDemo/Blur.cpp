#include <stdafx.h>
#include <Blur.h>

Blur::Blur(ID3D12Device* dev)
{
    _dev = dev;
}

void Blur::Init(std::pair<LPWSTR, LPWSTR> vsps)
{
    CreateRootSignature();
    ShaderCompile(vsps);
    SetInputLayout();
    CreateGraphicPipeline();

    RenderingSet();
}

// ルートシグネチャ設定
HRESULT Blur::CreateRootSignature()
{
    //サンプラー作成
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //ディスクリプタテーブルのスロット設定
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // shadow rendering
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // shadow rendering

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
HRESULT Blur::ShaderCompile(std::pair<LPWSTR, LPWSTR> vsps)
{
    auto pair = vsps;

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
void Blur::SetInputLayout()
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

        },

        //uv
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0, // スロットインデックス
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };
}

// パイプラインの生成
HRESULT Blur::CreateGraphicPipeline()
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
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE/*D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT*/;
    desc.DepthStencilState.DepthEnable = false;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget設定
void Blur::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
    SetGaussianData();
}

// RenderingTarget RTV,SRV,shadowRendering用ヒープの生成
HRESULT Blur::CreateRenderingHeap()
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
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    // shadowRendering用
    srvHeapDesc.NumDescriptors = 2;
    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(renderingHeap.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget用リソースの生成
HRESULT Blur::CreateRenderingResource()
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
    
    return result;
}

// RenderingTarget用RTVの作成
void Blur::CreateRenderingRTV()
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
void Blur::CreateRenderingSRV()
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

void Blur::SetGaussianData()
{
    auto weights = Utility::GetGaussianWeight(8, 5.0f);

    auto gaussianHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto gaussianBuffResDesc = CD3DX12_RESOURCE_DESC::Buffer(Utility::AlignmentSize(sizeof(weights[0]) * weights.size(), 256));
    _dev->CreateCommittedResource
    (
        &gaussianHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &gaussianBuffResDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(gaussianResource.ReleaseAndGetAddressOf())
    );

    auto handle = renderingHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    handle.ptr += inc;

    D3D12_CONSTANT_BUFFER_VIEW_DESC gDesc;
    gDesc.BufferLocation = gaussianResource->GetGPUVirtualAddress();
    gDesc.SizeInBytes = gaussianResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &gDesc,
        handle
    );

    gaussianResource->Map(0, nullptr, (void**)&mappedweight);
    std::copy(weights.begin(), weights.end(), mappedweight);
    gaussianResource->Unmap(0, nullptr);
}

void Blur::SetRenderingResourse(ComPtr<ID3D12Resource> _RenderingRsource)
{
    blurResource = _RenderingRsource;
    auto handle = renderingHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = /*DXGI_FORMAT_R8G8B8A8_UNORM*/blurResource.Get()->GetDesc().Format;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        blurResource.Get(),
        &srvDesc,
        handle
    );
}

// 実行
void Blur::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
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

    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _cmdList->OMSetRenderTargets(1, &heapHandle, false, nullptr);
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    _cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, renderingHeap.GetAddressOf());

    auto handle = renderingHeap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetGraphicsRootDescriptorTable(0, handle); // shadowMap

    handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(1, handle); // gaussian weights

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _cmdList->DrawInstanced(3, 1, 0, 0);

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);
}
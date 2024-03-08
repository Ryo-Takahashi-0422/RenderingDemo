#include <stdafx.h>
#include <Integration.h>

Integration::Integration(ID3D12Device* dev, ComPtr<ID3D12DescriptorHeap> heap)
{
    _dev = dev;
    _resourceManagerHeap = heap;
    buffSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    Init();
}

void Integration::Init()
{
    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();

    RenderingSet();
}

// ルートシグネチャ設定
HRESULT Integration::CreateRootSignature()
{
    //サンプラー作成
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //ディスクリプタテーブルのスロット設定
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // color thread1
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // color thread2
    descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); // depth thread1
    descTableRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // depth thread2
    descTableRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // sky 
    descTableRange[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); // imgui 
    descTableRange[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6); // sun  
    descTableRange[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7); // normal thread1
    descTableRange[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8); // normal thread2

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
    rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[2].DescriptorTable.pDescriptorRanges = &descTableRange[2];
    rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[3].DescriptorTable.pDescriptorRanges = &descTableRange[3];
    rootParam[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[4].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[4].DescriptorTable.pDescriptorRanges = &descTableRange[4];
    rootParam[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[5].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[5].DescriptorTable.pDescriptorRanges = &descTableRange[5];
    rootParam[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[6].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[6].DescriptorTable.pDescriptorRanges = &descTableRange[6];
    rootParam[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[7].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[7].DescriptorTable.pDescriptorRanges = &descTableRange[7];
    rootParam[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[8].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[8].DescriptorTable.pDescriptorRanges = &descTableRange[8];
    rootParam[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 9;
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
HRESULT Integration::ShaderCompile()
{
    std::string vs = "IntegrationVertex.hlsl";
    std::string ps = "IntegrationPixel.hlsl";
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
void Integration::SetInputLayout()
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
HRESULT Integration::CreateGraphicPipeline()
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
    desc.NumRenderTargets = 3;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1; //1サンプル/ピクセル
    desc.SampleDesc.Quality = 0;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE/*D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT*/;
    desc.DepthStencilState.DepthEnable = false;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget設定
void Integration::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
}

// RenderingTarget RTV,SRV,shadowRendering用ヒープの生成
HRESULT Integration::CreateRenderingHeap()
{
    // RTV用
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV用ディスクリプタヒープ
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 3;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
    if (result != S_OK)
        return result;

    // SRV用
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    srvHeapDesc.NumDescriptors = 3;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    //// shadowRendering用
    //srvHeapDesc.NumDescriptors = 2;
    //result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(renderingHeap.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget用リソースの生成
HRESULT Integration::CreateRenderingResource()
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
        IID_PPV_ARGS(renderingResource1.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &depthClearValue,
        IID_PPV_ARGS(renderingResource2.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &depthClearValue,
        IID_PPV_ARGS(renderingResource3.ReleaseAndGetAddressOf())
    );

    return result;
}

// RenderingTarget用RTVの作成
void Integration::CreateRenderingRTV()
{
    auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    _dev->CreateRenderTargetView
    (
        renderingResource1.Get(),
        &rtvDesc,
        handle
    );

    handle.ptr += inc;
    _dev->CreateRenderTargetView
    (
        renderingResource2.Get(),
        &rtvDesc,
        handle
    );

    handle.ptr += inc;
    _dev->CreateRenderTargetView
    (
        renderingResource3.Get(),
        &rtvDesc,
        handle
    );
}

// RenderingTarget用SRVの生成
void Integration::CreateRenderingSRV()
{
    auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        renderingResource1.Get(),
        &srvDesc,
        handle
    );

    handle.ptr += inc;
    _dev->CreateShaderResourceView
    (
        renderingResource2.Get(),
        &srvDesc,
        handle
    );

    handle.ptr += inc;
    _dev->CreateShaderResourceView
    (
        renderingResource3.Get(),
        &srvDesc,
        handle
    );
}

// 実行
void Integration::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
    auto barrierDesc1 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource1.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc1);

    auto barrierDesc2 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource2.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc2);

    auto barrierDesc3 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource3.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc3);

    D3D12_VIEWPORT viewport = *_viewPort;
    D3D12_RECT rect = *_rect;
    viewport.Width = width;
    viewport.Height = height;
    rect.right = width;
    rect.bottom = height;

    _cmdList->RSSetViewports(1, &viewport);
    _cmdList->RSSetScissorRects(1, &rect);

    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handles[3];
    uint32_t offset = 0;
    for (auto& handle : handles)
    {
        handle.InitOffsetted(heapHandle, inc * offset);
        offset += 1;
    }
    _cmdList->OMSetRenderTargets(3, handles, false, nullptr);

    //_cmdList->OMSetRenderTargets(1, &heapHandle, false, nullptr);
    //_cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);
    _cmdList->ClearRenderTargetView(handles[0], clearColor, 0, nullptr);
    _cmdList->ClearRenderTargetView(handles[1], clearColor, 0, nullptr);
    _cmdList->ClearRenderTargetView(handles[2], clearColor, 0, nullptr);

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, /*resourceManager[0]->GetSRVHeap()*/_resourceManagerHeap.GetAddressOf());

    auto gHandle = /*resourceManager[0]->GetSRVHeap()*/_resourceManagerHeap->GetGPUDescriptorHandleForHeapStart();
    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(0, gHandle); // sponza全体のレンダリング結果

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(1, gHandle); // connanのレンダリング結果

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(2, gHandle); //  sponza全体のデプスマップ

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(3, gHandle); // connanのデプスマップ

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(4, gHandle); // Sky

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(5, gHandle); // imgui

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(6, gHandle); // sun

    gHandle.ptr += buffSize * 4;
    _cmdList->SetGraphicsRootDescriptorTable(7, gHandle); // normal thread1

    gHandle.ptr += buffSize;
    _cmdList->SetGraphicsRootDescriptorTable(8, gHandle); // normal thread2

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _cmdList->DrawInstanced(3, 1, 0, 0);

    barrierDesc1 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource1.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc1);

    barrierDesc2 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource2.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc2);

    barrierDesc3 = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource3.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc3);
}
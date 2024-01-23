#include <stdafx.h>
#include <ShadowFactor.h>

// 確認用配列
std::vector<float>test(256, 0);

ShadowFactor::ShadowFactor(ID3D12Device* _dev, ID3D12Fence* _fence) : _dev(_dev), fence(_fence)
{
    Init();
    //CreateCommand();
}
ShadowFactor::~ShadowFactor()
{
    //D3D12_RANGE range{ 0, 1 };
    //participatingMediaResource->Unmap(0, &range);
}

// 初期化
void ShadowFactor::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreatePipeline();
    CreateHeap();
    CreateParticipatingMediaResource();
    CreateTextureResource();
    CreateView();
    Mapping();
}

// ルートシグネチャ設定
HRESULT ShadowFactor::CreateRootSignature()
{
    //サンプラー作成
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //ディスクリプタテーブルのスロット設定
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // martix
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // martix

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

// シェーダー設定
HRESULT ShadowFactor::ShaderCompile()
{
    //シェーダーコンパイル
    auto result = D3DCompileFromFile
    (
        L"ShadowFactor.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "cs_main",
        "cs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        csBlob.ReleaseAndGetAddressOf(),
        errorBlob.GetAddressOf()
    );

    //エラーチェック
    if (FAILED(result))
    {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            ::OutputDebugStringA("ファイルが見つかりません");
            csBlob = nullptr;
            return result;
        }
        else
        {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());

            std::copy_n((char*)errorBlob->GetBufferPointer(),
                errorBlob->GetBufferSize(),
                errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
    }

    return result;
}

// パイプラインの生成
HRESULT ShadowFactor::CreatePipeline()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.CS.pShaderBytecode = csBlob->GetBufferPointer();
    desc.CS.BytecodeLength = csBlob->GetBufferSize();
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask = 0;
    desc.pRootSignature = rootSignature.Get();

    auto result = _dev->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeLine));
    return result;
}

// ヒープの生成
HRESULT ShadowFactor::CreateHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 3; // CBV, UAV, SRV
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    return result;
}

// 関与媒質リソースの生成
HRESULT ShadowFactor::CreateParticipatingMediaResource()
{
    // ParticipatingMedia用
    auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ParticipatingMedia) + 0xff) & ~0xff);

    auto result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadヒープでのリソース初期状態はこのタイプが公式ルール
        nullptr,
        IID_PPV_ARGS(participatingMediaResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;
}

// 出力用テクスチャリソースの生成
HRESULT ShadowFactor::CreateTextureResource()
{
    CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    //Compute Shader出力用
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.Width = 256;
    textureDesc.Height = 256;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    auto result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&outputTextureResource)
    );
    if (result != S_OK) return result;

    // Compute Shaderが出力したテクスチャをコピーして、SRVとして扱うためのリソース
    //textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&copyTextureResource)
    );

    return result;
}

// ビューの生成
void ShadowFactor::CreateView()
{   
    auto handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // ParticipatingMedia用view
    D3D12_CONSTANT_BUFFER_VIEW_DESC pMediaCbvDesc = {};
    pMediaCbvDesc.BufferLocation = participatingMediaResource->GetGPUVirtualAddress();
    pMediaCbvDesc.SizeInBytes = participatingMediaResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &pMediaCbvDesc,
        handle
    );

    handle.ptr += inc;
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;

    _dev->CreateUnorderedAccessView
    (
        outputTextureResource.Get(),
        nullptr,
        &desc,
        handle
    );

    handle.ptr += inc;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        copyTextureResource.Get(),
        &srvDesc,
        handle
    );
}

// リソースのマップ
HRESULT ShadowFactor::Mapping()
{    
    // ParticipatingMedia
    auto result = participatingMediaResource->Map(0, nullptr, (void**)&m_Media);
    return result;
}

// 実行
UINT64 ShadowFactor::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList)
{
    //それぞれのセット
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipeLine);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetComputeRootDescriptorTable(0, handle);

    handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetComputeRootDescriptorTable(1, handle);

    _cmdList->Dispatch(16, 16, 1);

    // 出力用リソース状態をコピー元状態に設定
    auto barrierDescOfOutputTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        outputTextureResource.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_SOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfOutputTexture);

     // コピー用リソース状態をコピー先状態に設定
    auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        copyTextureResource.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_DEST
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

    _cmdList->CopyResource(copyTextureResource.Get(), outputTextureResource.Get());

    _cmdList->Close();

    //コマンドの実行
    ID3D12CommandList* executeList[] = { _cmdList };
    _cmdQueue->ExecuteCommandLists(1, executeList);

    UINT64 fenceVal = 0;
    HANDLE event; // fnece用イベント
    //-----ここでID3D12Fenceの待機をさせる-----
    _cmdQueue->Signal(fence.Get(), ++fenceVal);

    while (fence->GetCompletedValue() != fenceVal)
    {
        event = CreateEvent(nullptr, false, false, nullptr);
        fence->SetEventOnCompletion(fenceVal, event);
        //イベント発生待ち
        WaitForSingleObject(event, INFINITE);
        //イベントハンドルを閉じる
        CloseHandle(event);
    }

    _cmdAllocator->Reset();
    _cmdList->Reset(_cmdAllocator, nullptr);
    //GPUからデータをもらう
    //test.assign((float*)data, (float*)data + test.size());    
    return fenceVal;
 }

// 外部からの関与媒質設定
void ShadowFactor::SetParticipatingMedia(ParticipatingMedia media)
{
    m_Media = media;
}

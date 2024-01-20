#include <stdafx.h>
#include <ShadowFactor.h>

// 確認用配列
std::vector<float>test(256, 0);

Transmittance::Transmittance(ID3D12Device* _dev) :
    _dev(_dev), root(nullptr), shader(nullptr), pipe(nullptr), heap(nullptr), rsc(nullptr),
    data(nullptr), _cmdAllocator(nullptr), _cmdList(nullptr)
{
    Init();
    CreateCommand();
}
Transmittance::~Transmittance()
{
    D3D12_RANGE range{ 0, 1 };
    rsc->Unmap(0, &range);
}
// ルートシグネチャの生成
HRESULT Transmittance::CreateRoot()
{
    //シェーダーコンパイル
    auto hr = D3DCompileFromFile(L"Test.hlsl", nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "CS", "cs_5_1", D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shader, nullptr);
    //シェーダーからルートシグネチャ情報を取得
    ID3DBlob* sig = nullptr;
    hr = D3DGetBlobPart(shader->GetBufferPointer(), shader->GetBufferSize(),
        D3D_BLOB_ROOT_SIGNATURE, 0, &sig);
    //ルートシグネチャの生成
    hr = _dev->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(),
        IID_PPV_ARGS(&root));
    return hr;
}
// パイプラインの生成
HRESULT Transmittance::CreatePipe()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.CS.pShaderBytecode = shader->GetBufferPointer();
    desc.CS.BytecodeLength = shader->GetBufferSize();
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask = 0;
    desc.pRootSignature = root;

    auto hr = _dev->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipe));
    return hr;
}
// ヒープの生成
HRESULT Transmittance::CreateHeap()
{
    //今回はテストとしてリソースは1つとして作成
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto hr = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    return hr;
}
// リソースの生成
HRESULT Transmittance::CreateRsc()
{
    D3D12_HEAP_PROPERTIES prop{};
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    prop.CreationNodeMask = 1;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    prop.Type = D3D12_HEAP_TYPE_CUSTOM;
    prop.VisibleNodeMask = 1;
    //サイズは定数バッファと同じように指定
    D3D12_RESOURCE_DESC desc{};
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Height = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.MipLevels = 1;
    desc.SampleDesc = { 1, 0 };
    desc.Width = (sizeof(float) * test.size() + 0xff) & ~0xff;

    auto hr = _dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(&rsc));
        return hr;
}
// UAVの生成
void Transmittance::CreateUAV()
{
    //今回はGPUから受け取るのはchar型にしています
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Buffer.NumElements = test.size();
    desc.Buffer.StructureByteStride = sizeof(float);

    _dev->CreateUnorderedAccessView(rsc, nullptr, &desc, heap->GetCPUDescriptorHandleForHeapStart());
}
// リソースのマップ
HRESULT Transmittance::Map()
{
    D3D12_RANGE range{ 0, 1 };
    auto hr = rsc->Map(0, &range, &data);
    return hr;
}
// 初期化
void Transmittance::Init()
{
    CreateRoot();
    CreatePipe();
    CreateHeap();
    CreateRsc();
    CreateUAV();
    Map();
}
// コマンドの生成
HRESULT Transmittance::CreateCommand()
{
    auto hr = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
        IID_PPV_ARGS(&_cmdAllocator));
    hr = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, _cmdAllocator, nullptr,
        IID_PPV_ARGS(&_cmdList));
    return hr;
}
// 実行
void Transmittance::Execution(ID3D12CommandQueue* queue)
{
    //コマンドのリセット
    _cmdAllocator->Reset();
    _cmdList->Reset(_cmdAllocator, nullptr);
    //それぞれのセット
    _cmdList->SetComputeRootSignature(root);
    _cmdList->SetPipelineState(pipe);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetComputeRootDescriptorTable(0, handle);

    //コンピュートシェーダーの実行(今回は256個のスレッドグループを指定)
    _cmdList->Dispatch(test.size(), 1, 1);

    _cmdList->Close();

    //コマンドの実行
    ID3D12CommandList* executeList[] = { _cmdList };
    queue->ExecuteCommandLists(1, executeList);

    
    //-----ここでID3D12Fenceの待機をさせる-----
    

    //GPUからデータをもらう
    test.assign((float*)data, (float*)data + test.size());
 }
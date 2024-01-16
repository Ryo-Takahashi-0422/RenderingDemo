#pragma once

class SkyLUT
{
private:
    // ルートシグネチャの生成
    HRESULT CreateRoot();
    // パイプラインの生成
    HRESULT CreatePipe();
    // ヒープの生成
    HRESULT CreateHeap();
    // リソースの生成
    HRESULT CreateRsc();
    // RTVの生成
    void CreateRTV();
    // SRVの生成
    void CreateSRV();
    // リソースのマップ
    HRESULT Map();
    // 初期化
    void Init();
    // コマンドの生成
    HRESULT CreateCommand();


    // デバイス
    ComPtr<ID3D12Device> _dev;
    // コンピュート用ルートシグネチャ
    ComPtr<ID3D12RootSignature> root;
    // シェーダー情報
    ComPtr<ID3DBlob> shader;
    // コンピュート用パイプライン
    ComPtr<ID3D12PipelineState> pipe;
    // ヒープ
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    // リソース
    ComPtr<ID3D12Resource> rsc;
    // 送受信用データ
    void* data;
    // コマンドアロケータ
    ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    // コマンドリスト
    ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width = 64;
    UINT64 height = 64;

public:
    SkyLUT(ID3D12Device* dev);
    ~SkyLUT();

    // 実行
    void Execution(ID3D12CommandQueue* cmdQueue);

};
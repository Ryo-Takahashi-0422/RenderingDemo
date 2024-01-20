#pragma once

class Transmittance
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
    // UAVの生成
    void CreateUAV();
    // リソースのマップ
    HRESULT Map();
    // 初期化
    void Init();
    // コマンドの生成
    HRESULT CreateCommand();


    // デバイス
    ID3D12Device* _dev;
    // コンピュート用ルートシグネチャ
    ID3D12RootSignature* root;
    // シェーダー情報
    ID3DBlob* shader;
    // コンピュート用パイプライン
    ID3D12PipelineState* pipe;
    // ヒープ
    ID3D12DescriptorHeap* heap;
    // リソース
    ID3D12Resource* rsc;
    // 送受信用データ
    void* data;
    // コマンドアロケータ
    ID3D12CommandAllocator* _cmdAllocator;
    // コマンドリスト
    ID3D12GraphicsCommandList* _cmdList;

public:
    Transmittance(ID3D12Device* dev);
    ~Transmittance();

    // 実行
    void Execution(ID3D12CommandQueue* cmdQueue);

};
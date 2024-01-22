#pragma once

class ShadowFactor
{
private:
    // ルートシグネチャの生成
    HRESULT CreateRootSignature();
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[2] = {};
    D3D12_ROOT_PARAMETER rootParam[2] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
    ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
    // シェーダー設定
    HRESULT ShaderCompile();
    // パイプラインの生成
    HRESULT CreatePipeline();
    // ヒープの生成
    HRESULT CreateHeap();
    // 関与媒質リソースの生成
    HRESULT CreateParticipatingMediaResource();
    // 出力用テクスチャリソースの生成
    HRESULT CreateOutputTextureResource();
    // ビューの生成
    void CreateView();
    // リソースのマップ
    HRESULT Mapping();
    // 初期化
    void Init();
    // コマンドの生成
    HRESULT CreateCommand();
    // 関与媒質マッピング先
    ParticipatingMedia m_Media;

    // デバイス
    ID3D12Device* _dev = nullptr;
    // コンピュート用ルートシグネチャ
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // シェーダー情報
    ComPtr<ID3D10Blob> csBlob = nullptr; // 頂点シェーダーオブジェクト格納用
    // コンピュート用パイプライン
    ID3D12PipelineState* pipeLine = nullptr;
    // ヒープ
    ID3D12DescriptorHeap* heap = nullptr;
    // リソース
    ComPtr<ID3D12Resource> participatingMediaResource = nullptr;
    ComPtr<ID3D12Resource> outputTextureResource = nullptr;
    // 送受信用データ
    void* data = nullptr;
    // コマンドアロケータ
    ID3D12CommandAllocator* _cmdAllocator = nullptr;
    // コマンドリスト
    ID3D12GraphicsCommandList* _cmdList = nullptr;

    ComPtr<ID3D11Texture2D> texture = nullptr;

public:
    ShadowFactor(ID3D12Device* dev);
    ~ShadowFactor();
    void SetParticipatingMedia(ParticipatingMedia media);

    // 実行
    void Execution(ID3D12CommandQueue* cmdQueue);

};
#pragma once

struct SkyLUTBuffer
{
    XMFLOAT3 eyePos;
    XMFLOAT3 sunDirection; // 
    float stepCnt;
    XMFLOAT3 sunIntensity;
};

class SkyLUT
{
private:

    // 初期化
    void Init();

    // ルートシグネチャの生成
    HRESULT CreateRootSignature();
    // シェーダー設定
    HRESULT ShaderCompile();
    //
    void SetInputLayout();
   /* std::vector<*/D3D12_INPUT_ELEMENT_DESC/*>*/ inputLayout[2];
    // パイプラインの生成
    HRESULT CreateGraphicPipeline();

    // RenderingTargetの設定
    void RenderingSet();
    // RenderingTarget用ヒープの生成
    HRESULT CreateRenderingHeap();
    // RenderingTarget用リソースの生成
    HRESULT CreateRenderingResource();
    // RenderingTarget用RTVの生成
    void CreateRenderingRTV();
    // RenderingTarget用SRVの生成
    void CreateRenderingSRV();

    // 関与媒質の設定
    void InitParticipatingMedia();
    // 関与媒質用リソースの生成
    HRESULT CreateParticipatingResource();
    ComPtr<ID3D12Resource> participatingMediaResource;
    ComPtr<ID3D12Resource> skyLUTBufferResource;
    // 関与媒質用ヒープ・ビューの生成
    HRESULT CreateParticipatingMediaHeapAndView();
    // 関与媒質用定数のマッピング
    void MappingParticipatingMedia();
    // マッピング先
    ParticipatingMedia m_Media;
    SkyLUTBuffer m_SkyLUT;
    // コマンドの生成
    HRESULT CreateCommand();

    // デバイス
    ComPtr<ID3D12Device> _dev;
    // ルートシグネチャ関連
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[1] = {};
    D3D12_ROOT_PARAMETER rootParam[1] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
    ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
    ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
    ComPtr<ID3DBlob> _errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // シェーダー情報
    ComPtr<ID3DBlob> shader;
    // コンピュート用パイプライン
    ComPtr<ID3D12PipelineState> pipelineState;
    // ヒープ
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> skyLUTHeap;
    // リソース
    ComPtr<ID3D12Resource> renderingResource;
    
    // 送受信用データ
    void* data;
    // コマンドアロケータ
    ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    // コマンドリスト
    ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width = 64;
    UINT64 height = 64;

public:
    SkyLUT();
    SkyLUT(ID3D12Device* dev);
    ~SkyLUT();
    void SetParticipatingMedia(ParticipatingMedia media);
    void SetSkyLUTBuffer(SkyLUTBuffer buffer);

    // 実行
    void Execution(ID3D12CommandQueue* cmdQueue);

};
#pragma once

struct SkyLUTBuffer
{
    //https://learn.microsoft.com/ja-jp/windows/win32/dxmath/pg-xnamath-optimizing XMFLOAT3は4byteアラインメントだが、実質16byteアラインメントされている
    XMFLOAT3 eyePos;
    float pad0;
    XMFLOAT3 sunDirection;    
    float stepCnt;
    XMFLOAT3 sunIntensity;
    int width;
    int height;
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
    void CreateRenderingCBVSRV();

    // 関与媒質の設定
    void InitParticipatingMedia();
    // 関与媒質用リソースの生成
    HRESULT CreateParticipatingResource();
    ComPtr<ID3D12Resource> participatingMediaResource;
    ComPtr<ID3D12Resource> skyLUTBufferResource;
    ComPtr<ID3D12Resource> shadowFactorResource;

    // 関与媒質用ヒープ・ビューの生成
    HRESULT CreateParticipatingMediaHeapAndView();
    // 関与媒質用定数のマッピング
    void MappingParticipatingMedia();
    // マッピング先
    ParticipatingMedia* m_Media = nullptr;
    SkyLUTBuffer* m_SkyLUT = nullptr;

    // デバイス
    ComPtr<ID3D12Device> _dev;
    // フェンス
    //ComPtr<ID3D12Fence> fence = nullptr;
    // ルートシグネチャ関連
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[4] = {};
    D3D12_ROOT_PARAMETER rootParam[4] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
    ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
    ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
    ComPtr<ID3DBlob> _errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // シェーダー情報
    //ComPtr<ID3DBlob> shader;
    // コンピュート用パイプライン
    ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    // ヒープ
    ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> cbvsrvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> skyLUTHeap = nullptr;
    // リソース
    ComPtr<ID3D12Resource> renderingResource;
    
    // 送受信用データ
    void* data;
    //// コマンドアロケータ
    //ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    //// コマンドリスト
    //ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width;
    UINT64 height;

    void RecreatreSource();
    bool barrierSW = true; // シャドウファクターの解像度変化および初回の描画時にのみtrue

public:
    //SkyLUT();
    SkyLUT(ID3D12Device* dev, ID3D12Resource* _shadowFactorRsource, int _width, int _height);
    ~SkyLUT();
    void SetParticipatingMedia(ParticipatingMedia media);
    void SetSkyLUTBuffer(SkyLUTBuffer buffer);
    void SetShadowFactorResource(ID3D12Resource* _shadowFactorRsource);
    void SetSkyLUTResolution();
    void ChangeSkyLUTResolution(int _width, int _height);
    void SetBarrierSWTrue();
    
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    ComPtr<ID3D12DescriptorHeap> GetSkyLUTRenderingHeap() { return cbvsrvHeap; };
    ComPtr<ID3D12Resource> GetSkyLUTRenderingResource() { return renderingResource; };
};
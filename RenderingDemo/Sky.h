#pragma once

class Sky
{
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

    // Frustum関連
    void InitFrustumReosources();
    HRESULT CreateFrustumHeap();
    HRESULT CreateFrustumResource();
    void CreateSkyView();
    HRESULT MappingFrustum();


    // デバイス
    ComPtr<ID3D12Device> _dev;
    // フェンス
    ComPtr<ID3D12Fence> fence = nullptr;
    // ルートシグネチャ関連
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[2] = {};
    D3D12_ROOT_PARAMETER rootParam[2] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
    ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
    ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
    ComPtr<ID3DBlob> _errorBlob = nullptr; // シェーダー関連エラー格納用
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // シェーダー情報
    //ComPtr<ID3DBlob> shader;
    // コンピュート用パイプライン
    ComPtr<ID3D12PipelineState> pipelineState;
    // ヒープ
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> skyHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
    // リソース
    ComPtr<ID3D12Resource> renderingResource;
    ComPtr<ID3D12Resource> frustumResource;
    ComPtr<ID3D12Resource> skyLUTResource;

    float resWidth = 256;
    float resHeight = 256;

    Frustum* m_Frustum = nullptr;

public:
    Sky();
    Sky(ID3D12Device* dev, ID3D12Fence* _fence, ID3D12Resource* _skyLUTRsource);
    ~Sky();
    void SetFrustum(Frustum _frustum);
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    void SetSkyLUTResource();
    ComPtr<ID3D12DescriptorHeap> GetSkyLUTRenderingHeap() { return rtvHeap; };
    ComPtr<ID3D12Resource> GetSkyLUTRenderingResource() { return renderingResource; };
};
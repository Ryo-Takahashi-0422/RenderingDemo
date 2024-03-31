#pragma once

class PreFxaa
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

    void CreateInfoViewAndMapping();

    // デバイス
    ComPtr<ID3D12Device> _dev = nullptr;

    // ルートシグネチャ関連
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[6] = {};
    D3D12_ROOT_PARAMETER rootParam[6] = {};
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
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // レンダリング結果格納
    ComPtr<ID3D12DescriptorHeap> externalHeap = nullptr; // integrationリソース格納用

    // リソース
    ComPtr<ID3D12Resource> renderingResource = nullptr; // レンンダリング結果

    ComPtr<ID3D12Resource> colorResource = nullptr; // レンダーターゲット1 カラー画像
    ComPtr<ID3D12Resource> normalResource = nullptr; // レンダーターゲット2 法線画像
    ComPtr<ID3D12Resource> imguiResource = nullptr; // レンダーターゲット3 imgui画像
    ComPtr<ID3D12Resource> bluedColorResourse = nullptr; // 外部リソース2 blured color
    ComPtr<ID3D12Resource> depthResourse = nullptr; // 外部リソース3 depth
    ComPtr<ID3D12Resource> infoResourse = nullptr; // 外部リソース3 depth

    float width;
    float height;

    struct Infos
    {
        float size;
        bool isFxaa;
    };
    Infos* infos = nullptr;

public:
    PreFxaa(ID3D12Device* dev, int _width, int _height);
    ~PreFxaa();
    //void SetSRHeapAndSRV(ComPtr<ID3D12DescriptorHeap> srvHeap);
    void Execution(ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    void SetResourse1(ComPtr<ID3D12Resource> _resource);
    void SetResourse2(ComPtr<ID3D12Resource> _resource);
    void SetResourse3(ComPtr<ID3D12Resource> _resource);
    void SetResourse4(ComPtr<ID3D12Resource> _resource);
    void SetResourse5(ComPtr<ID3D12Resource> _resource);
    void SetExternalView();

    ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return srvHeap; };
    ComPtr<ID3D12Resource> GetRenderingResource() { return renderingResource; };
};
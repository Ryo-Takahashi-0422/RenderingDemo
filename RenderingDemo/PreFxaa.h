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

    // デバイス
    ComPtr<ID3D12Device> _dev = nullptr;

    // ルートシグネチャ関連
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[5] = {};
    D3D12_ROOT_PARAMETER rootParam[5] = {};
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
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> extSrvHeap = nullptr;
    // リソース
    ComPtr<ID3D12Resource> renderingResource = nullptr;

    float width;
    float height;

    Integration* m_integration = nullptr;

public:
    PreFxaa(ID3D12Device* dev, Integration* _integration, int _width, int _height);
    ~PreFxaa();
    //void SetSRHeapAndSRV(ComPtr<ID3D12DescriptorHeap> srvHeap);
    void Execution(ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return srvHeap; };
    ComPtr<ID3D12Resource> GetRenderingResource() { return renderingResource; };
};
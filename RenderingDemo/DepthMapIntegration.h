#pragma once

class DepthMapIntegration
{
private:
    // 解像度関連
    int width;
    int height;
    int threadIdNum_X = 16;
    int threadIdNum_Y = 16;

    // ルートシグネチャの生成
    HRESULT CreateRootSignature();
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[3] = {};
    D3D12_ROOT_PARAMETER rootParam[3] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
    ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
    // シェーダー設定
    HRESULT ShaderCompile();
    // パイプラインの生成
    HRESULT CreatePipeline();
    // ヒープの生成
    HRESULT CreateHeap();
    // 出力用テクスチャリソースの生成
    HRESULT CreateTextureResource();
    // ビューの生成
    void CreateView();

    // 初期化
    void Init();

    // デバイス
    ID3D12Device* _dev = nullptr;
    // フェンス
    ComPtr<ID3D12Fence> fence = nullptr;
    // コンピュート用ルートシグネチャ
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // シェーダー情報
    ComPtr<ID3D10Blob> csBlob = nullptr; // 頂点シェーダーオブジェクト格納用
    // コンピュート用パイプライン
    ID3D12PipelineState* pipeLine = nullptr;
    // ヒープ
    ID3D12DescriptorHeap* heap = nullptr;
    // リソース
    ComPtr<ID3D12Resource> depthmapResource1 = nullptr;
    ComPtr<ID3D12Resource> depthmapResource2 = nullptr;
    ComPtr<ID3D12Resource> outputTextureResource = nullptr;
    ComPtr<ID3D12Resource> copyTextureResource = nullptr;
    // コマンドアロケータ
    ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
    // コマンドリスト
    ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;

public:
    DepthMapIntegration(ID3D12Device* dev, ComPtr<ID3D12Resource> _depthmapResource1, ComPtr<ID3D12Resource> _depthmapResource2, int _width, int _height);
    ~DepthMapIntegration();
    ComPtr<ID3D12Resource> GetTextureResource() { return copyTextureResource; };
    // 実行
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList);
    void ChangeResolution(int _width, int _height);
};
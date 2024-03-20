#pragma once

class Blur
{
private:

	// デバイス
	ComPtr<ID3D12Device> _dev;
	// ルートシグネチャ関連
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[3] = {};
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
	ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
	ComPtr<ID3DBlob> _errorBlob = nullptr; // シェーダー関連エラー格納用
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// コンピュート用パイプライン
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ルートシグネチャの生成
	HRESULT CreateRootSignature();
	// シェーダー設定
	HRESULT ShaderCompile(std::pair<LPWSTR, LPWSTR> vsps);
	void SetInputLayout();
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	// パイプラインの生成
	HRESULT CreateGraphicPipeline();

	// RenderingTargetの設定
	void RenderingSet();
	// ヒープの生成
	HRESULT CreateRenderingHeap();
	// リソースの生成
	HRESULT CreateRenderingResource();
	// RTV生成
	void CreateRenderingRTV();
	// SRV生成
	void CreateRenderingSRV();
	// gaussian weight
	void SetGaussianData();

	ComPtr<ID3D12Resource> renderingResource = nullptr; // 描画用
	ComPtr<ID3D12Resource> blurResource = nullptr; // ブラー対象リソース
	ComPtr<ID3D12Resource> gaussianResource = nullptr;
	ComPtr<ID3D12Resource> switchResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> renderingHeap = nullptr; // rendering用ディスクリプタヒープ

	float width/* = 4096*/;
	float height/* = 4096*/;

	float* mappedweight = nullptr;
	bool* mappedSwitch = nullptr;

	void RecreatreSource();

public:
	Blur(ID3D12Device* dev);
	~Blur();
	void Init(std::pair<LPWSTR, LPWSTR> vsps, std::pair<float, float> resolution);
	void SetRenderingResourse(ComPtr<ID3D12Resource> _renderingRsource);
	void SetSwitch(bool _switch);
	void ChangeSwitch(bool _switch);
	ComPtr<ID3D12Resource> GetBlurResource() { return renderingResource; };
	void ChangeSceneResolution(int _width, int _height);
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};
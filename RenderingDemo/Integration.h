#pragma once

class Integration
{
private:

	// デバイス
	ComPtr<ID3D12Device> _dev;
	// ルートシグネチャ関連
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[9] = {};
	D3D12_ROOT_PARAMETER rootParam[9] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ルートシグネチャオブジェクト格納用
	ComPtr<ID3DBlob> errorBlob = nullptr; // シェーダー関連エラー格納用
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // 頂点シェーダーオブジェクト格納用
	ComPtr<ID3D10Blob> _psBlob = nullptr; // ピクセルシェーダーオブジェクト格納用
	ComPtr<ID3DBlob> _errorBlob = nullptr; // シェーダー関連エラー格納用
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ルートシグネチャの生成
	HRESULT CreateRootSignature();
	// シェーダー設定
	HRESULT ShaderCompile();
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

	ComPtr<ID3D12Resource> renderingResource1 = nullptr; // レンダーターゲット1 カラー画像
	ComPtr<ID3D12Resource> renderingResource2 = nullptr; // レンダーターゲット2 法線画像
	ComPtr<ID3D12Resource> renderingResource3 = nullptr; // レンダーターゲット3 imgui画像
	ComPtr<ID3D12Resource> shaderResourse1 = nullptr; // 外部リソース1 ssao
	ComPtr<ID3D12Resource> shaderResourse2 = nullptr; // 外部リソース2 blured color
	ComPtr<ID3D12Resource> shaderResourse3 = nullptr; // 外部リソース3 depth

	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> _resourceManagerHeap = nullptr; // rendering用ディスクリプタヒープ

	float width;
	float height;
	UINT buffSize;
	float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };

public:
	Integration(ID3D12Device* dev, ComPtr<ID3D12DescriptorHeap> heap, int _width, int _height);
	~Integration();
	void Init();
	void SetResourse1(ComPtr<ID3D12Resource> _resource);
	void SetResourse2(ComPtr<ID3D12Resource> _resource);
	void SetResourse3(ComPtr<ID3D12Resource> _resource);
	ComPtr<ID3D12Resource> GetColorResourse() { return  renderingResource1; };
	ComPtr<ID3D12Resource> GetNormalResourse() { return  renderingResource2; };
	ComPtr<ID3D12Resource> GetImguiResourse() { return  renderingResource3; };
	ComPtr<ID3D12DescriptorHeap> GetSRVHeap() { return srvHeap; };
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
	std::pair<float, float> GetResolution() { return std::pair<float, float>(width, height); };
};
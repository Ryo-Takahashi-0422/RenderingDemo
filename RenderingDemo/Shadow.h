#pragma once

class Shadow
{
private:

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
	// コンピュート用パイプライン
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	std::vector<D3D12_VERTEX_BUFFER_VIEW*> vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW*> ibViews;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> itIndiceFirsts;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>> indiceContainer;

	// ルートシグネチャの生成
	HRESULT CreateRootSignature();
	// シェーダー設定
	HRESULT ShaderCompile();	//
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
	// DSV生成
	void CreateRenderingDSV();

	ComPtr<ID3D12Resource> renderingResource = nullptr; // 描画用
	ComPtr<ID3D12Resource> depthBuff = nullptr; // depth buffer
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ

	struct WVPMatrix
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX rotation;
		XMMATRIX bones[256];
		XMFLOAT3 lightPos;
	};
	// VPmatrix関連
	void InitWVPMatrixReosources();
	HRESULT CreateWVPMatrixHeap();
	HRESULT CreateWVPMatrixResources();
	void CreateWVPMatrixView();
	HRESULT MappingWVPMatrix();

	ComPtr<ID3D12Resource> matrixResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // WVPmatrix用ディスクリプタヒープ
	WVPMatrix* mappedMatrix = nullptr;
	XMMATRIX m_moveMatrix = XMMatrixIdentity();
	XMMATRIX m_rotationMatrix = XMMatrixIdentity();

	float width;
	float height;

	void UpdateWorldMatrix();
	

public:
	Shadow(ID3D12Device* dev, int _width, int _height);
	~Shadow();
	void Init();
	void SetVertexAndIndexInfo(std::vector<D3D12_VERTEX_BUFFER_VIEW*> _vbViews, std::vector<D3D12_INDEX_BUFFER_VIEW*> _ibViews, std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> _itIndiceFirsts, std::vector<std::vector<std::pair<std::string, VertexInfo>>> _indiceContainer);
	void SetVPMatrix(XMMATRIX _sunView, XMMATRIX _sunProj);
	void SetSunPos(XMFLOAT3 _sunPos);
	void SetMoveMatrix(XMMATRIX charaWorldMatrix);
	void SetRotationMatrix(XMMATRIX rotationMatrix);
	void SetBoneMatrix(FBXSceneMatrix* _fbxSceneMatrix);
	ComPtr<ID3D12Resource> GetShadowMapResource() { return depthBuff; };
	ComPtr<ID3D12Resource> GetShadowRenderingResource() { return renderingResource; };
	XMMATRIX GetShadowPosMatrix() { return mappedMatrix->world; };
	XMMATRIX GetShadowPosInvMatrix() { return XMMatrixInverse(nullptr, mappedMatrix->world); };
	XMMATRIX GetShadowView() { return mappedMatrix->view; };
	std::pair<float, float> GetResolution() { return std::pair<float, float>(width, height); };
	//void ChangeSceneResolution(int _width, int _height);
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};
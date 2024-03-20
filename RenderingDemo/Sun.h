#pragma once

class Sun
{
private:

	// デバイス
	ComPtr<ID3D12Device> _dev;
	Camera* _camera = nullptr;
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
	// コンピュート用パイプライン
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ルートシグネチャの生成
	HRESULT CreateRootSignature();
	// シェーダー設定
	HRESULT ShaderCompile();	//
	void SetInputLayout();
	/* std::vector<*/D3D12_INPUT_ELEMENT_DESC/*>*/ inputLayout[2];
	// パイプラインの生成
	HRESULT CreateGraphicPipeline();

	void CreateSunVertex();
	int vertexCnt = 192;
	std::vector<XMVECTOR> vertexes;
	std::vector<unsigned int> indices;
	XMFLOAT3 direction;

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

	// 頂点・インデックス関連
	void DrawResourceSet();
	HRESULT CreateDrawResourceResource();
	void CreateDrawResourceView();
	HRESULT MappingDrawResource();

	// billboardmatrix関連
	void InitBillboardMatrixReosources();
	HRESULT CreateBillboardMatrixHeap();
	HRESULT CreateBillboardMatrixResources();
	void CreateBillboardMatrixView();
	HRESULT MappingBillboardMatrix();

	ComPtr<ID3D12Resource> renderingResource = nullptr;
	ComPtr<ID3D12Resource> matrixResource = nullptr;
	ComPtr<ID3D12Resource> shadowFactorResource;
	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // billboardmatrix用ディスクリプタヒープ
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertexビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Indexビュー
	XMVECTOR* mappedVertPos = nullptr;
	unsigned int* mappedIdx = nullptr;
	float width = 1024;
	float height = 1024;

	struct BillboardMatrix
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX cameraPos;
		XMMATRIX sunDir;
		XMMATRIX billborad;
		XMMATRIX scene;
		XMFLOAT3 sunTheta;
	};
	BillboardMatrix* mappedMatrix = nullptr;
	XMMATRIX sceneMatrix = XMMatrixIdentity();
	XMMATRIX sunViewMatrix;
	XMMATRIX shadowViewMatrix;
	XMMATRIX sunProjMatrix;
	XMFLOAT3 expFixedDir;
	float adjustDirValue = 95.0f; // 観測値として65以下だと平行投影ビュー時に太陽位置がある角度においてsponzaにめり込み、シェーディングエラーが発生する

public:
	Sun(ID3D12Device* dev, Camera* camera);
	~Sun();
	void Init();
	void CalculateBillbordMatrix();
	XMFLOAT3 CalculateDirectionFromDegrees(float angleX, float angleY);
	void CalculateViewMatrix();
	XMFLOAT3 GetDirection() { return direction; };
	XMFLOAT3 GetFixedDirection() { return expFixedDir; };

	void ChangeSceneMatrix(XMMATRIX _world);
	ComPtr<ID3D12Resource> GetRenderResource() { return renderingResource; };
	XMMATRIX GetViewMatrix() { return sunViewMatrix; };
	XMMATRIX GetShadowViewMatrix() { return shadowViewMatrix; };
	XMMATRIX GetProjMatrix() { return sunProjMatrix; };
	void SetShadowFactorResource(ID3D12Resource* _shadowFactorRsource);
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};

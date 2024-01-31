#pragma once

class Sun
{
private:

	// �f�o�C�X
	ComPtr<ID3D12Device> _dev;
	// ���[�g�V�O�l�`���֘A
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[3] = {};
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> _errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// �R���s���[�g�p�p�C�v���C��
	ComPtr<ID3D12PipelineState> pipelineState;

	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSignature();
	// �V�F�[�_�[�ݒ�
	HRESULT ShaderCompile();	//
	void SetInputLayout();
	/* std::vector<*/D3D12_INPUT_ELEMENT_DESC/*>*/ inputLayout[2];
	// �p�C�v���C���̐���
	HRESULT CreateGraphicPipeline();

	void CreateSunVertex();
	int vertexCnt = 48;
	std::vector<XMVECTOR> vertexes;
	std::vector<unsigned int> indices;
	XMFLOAT3 direction;

	// RenderingTarget�̐ݒ�
	void RenderingSet();
	// RenderingTarget�p�q�[�v�̐���
	HRESULT CreateRenderingHeap();
	// RenderingTarget�p���\�[�X�̐���
	HRESULT CreateRenderingResource();
	// RenderingTarget�pRTV�̐���
	void CreateRenderingRTV();
	// RenderingTarget�pSRV�̐���
	void CreateRenderingSRV();

	// ���_�E�C���f�b�N�X�֘A
	void DrawResourceSet();
	HRESULT CreateDrawResourceResource();
	void CreateDrawResourceView();
	HRESULT MappingDrawResource();

	// billboardmatrix�֘A
	void InitBillboardMatrixReosources();
	HRESULT CreateBillboardMatrixHeap();
	HRESULT CreateBillboardMatrixResources();
	void CreateBillboardMatrixView();
	HRESULT MappingBillboardMatrix();

	ComPtr<ID3D12Resource> renderingResource = nullptr;
	ComPtr<ID3D12Resource> matrixResource = nullptr;
	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // billboardmatrix�p�f�B�X�N���v�^�q�[�v
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertex�r���[
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Index�r���[
	XMVECTOR* mappedVertPos = nullptr;
	unsigned int* mappedIdx = nullptr;
	float width = 64;
	float height = 64;

	struct BillboardMatrix
	{
		XMMATRIX matrix;
	};
	BillboardMatrix* billboardMatrix = nullptr;

public:

	void Init();
	XMMATRIX CalculateBillbordMatrix();
	XMFLOAT3 CalculateDirectionFromDegrees(float angleX, float angleY);
	XMFLOAT3 GetDirection() { return direction; };
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};

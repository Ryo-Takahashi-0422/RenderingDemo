#pragma once

class Sun
{
private:

	// �f�o�C�X
	ComPtr<ID3D12Device> _dev;
	Camera* _camera = nullptr;
	// ���[�g�V�O�l�`���֘A
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[2] = {};
	D3D12_ROOT_PARAMETER rootParam[2] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> _errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// �R���s���[�g�p�p�C�v���C��
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSignature();
	// �V�F�[�_�[�ݒ�
	HRESULT ShaderCompile();	//
	void SetInputLayout();
	/* std::vector<*/D3D12_INPUT_ELEMENT_DESC/*>*/ inputLayout[2];
	// �p�C�v���C���̐���
	HRESULT CreateGraphicPipeline();

	void CreateSunVertex();
	int vertexCnt = 192;
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
	ComPtr<ID3D12Resource> shadowFactorResource;
	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // billboardmatrix�p�f�B�X�N���v�^�q�[�v
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertex�r���[
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Index�r���[
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
	float adjustDirValue = 95.0f; // �ϑ��l�Ƃ���65�ȉ����ƕ��s���e�r���[���ɑ��z�ʒu������p�x�ɂ�����sponza�ɂ߂荞�݁A�V�F�[�f�B���O�G���[����������

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

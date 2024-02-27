#pragma once

class Shadow
{
private:

	// �f�o�C�X
	ComPtr<ID3D12Device> _dev;
	// ���[�g�V�O�l�`���֘A
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[1] = {};
	D3D12_ROOT_PARAMETER rootParam[1] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> _errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// �R���s���[�g�p�p�C�v���C��
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	std::vector<D3D12_VERTEX_BUFFER_VIEW*> vbViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW*> ibViews;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> itIndiceFirsts;
	std::vector<std::vector<std::pair<std::string, VertexInfo>>> indiceContainer;

	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSignature();
	// �V�F�[�_�[�ݒ�
	HRESULT ShaderCompile();	//
	void SetInputLayout();
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	// �p�C�v���C���̐���
	HRESULT CreateGraphicPipeline();

	// RenderingTarget�̐ݒ�
	void RenderingSet();
	// �q�[�v�̐���
	HRESULT CreateRenderingHeap();
	// ���\�[�X�̐���
	HRESULT CreateRenderingResource();
	// RTV����
	void CreateRenderingRTV();
	// SRV����
	void CreateRenderingSRV();
	// DSV����
	void CreateRenderingDSV();

	ComPtr<ID3D12Resource> renderingResource = nullptr; // �`��p
	ComPtr<ID3D12Resource> depthBuff = nullptr; // depth buffer
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // �[�x�X�e���V���r���[�p�f�B�X�N���v�^�q�[�v

	struct WVPMatrix
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX rotation;
		XMMATRIX bones[256];
		XMFLOAT3 lightPos;
	};
	// VPmatrix�֘A
	void InitWVPMatrixReosources();
	HRESULT CreateWVPMatrixHeap();
	HRESULT CreateWVPMatrixResources();
	void CreateWVPMatrixView();
	HRESULT MappingWVPMatrix();

	ComPtr<ID3D12Resource> matrixResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // WVPmatrix�p�f�B�X�N���v�^�q�[�v
	WVPMatrix* mappedMatrix = nullptr;
	XMMATRIX m_moveMatrix = XMMatrixIdentity();
	XMMATRIX m_rotationMatrix = XMMatrixIdentity();

	float width = 4096;
	float height = 4096;

	void UpdateWorldMatrix();

public:
	Shadow(ID3D12Device* dev);
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
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};
#pragma once

class Integration
{
private:

	// �f�o�C�X
	ComPtr<ID3D12Device> _dev;
	// ���[�g�V�O�l�`���֘A
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[9] = {};
	D3D12_ROOT_PARAMETER rootParam[9] = {};
	ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
	ComPtr<ID3DBlob> _errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSignature();
	// �V�F�[�_�[�ݒ�
	HRESULT ShaderCompile();
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

	ComPtr<ID3D12Resource> renderingResource1 = nullptr; // �����_�[�^�[�Q�b�g1 �J���[�摜
	ComPtr<ID3D12Resource> renderingResource2 = nullptr; // �����_�[�^�[�Q�b�g2 �@���摜
	ComPtr<ID3D12Resource> renderingResource3 = nullptr; // �����_�[�^�[�Q�b�g3 �f�v�X�摜
	ComPtr<ID3D12Resource> integratedDepthmap = nullptr; // �R���s���[�g�V�F�[�_�[�ɂ�蓝�����ꂽ�O�����\�[�X

	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> _resourceManagerHeap = nullptr; // rendering�p�f�B�X�N���v�^�q�[�v

	float width = 1024;
	float height = 1024;
	UINT buffSize;
	float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };

public:
	Integration(ID3D12Device* dev, ComPtr<ID3D12DescriptorHeap> heap);
	void Init();
	void SetDepthmapResourse(ComPtr<ID3D12Resource> _resource);
	ComPtr<ID3D12Resource> GetNormalResourse() { return  renderingResource2; };
	ComPtr<ID3D12DescriptorHeap> GetSRVHeap() { return srvHeap; };
	void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);
};
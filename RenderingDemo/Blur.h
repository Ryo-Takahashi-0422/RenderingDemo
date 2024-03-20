#pragma once

class Blur
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
	ComPtr<ID3D12PipelineState> pipelineState = nullptr;

	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSignature();
	// �V�F�[�_�[�ݒ�
	HRESULT ShaderCompile(std::pair<LPWSTR, LPWSTR> vsps);
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
	// gaussian weight
	void SetGaussianData();

	ComPtr<ID3D12Resource> renderingResource = nullptr; // �`��p
	ComPtr<ID3D12Resource> blurResource = nullptr; // �u���[�Ώۃ��\�[�X
	ComPtr<ID3D12Resource> gaussianResource = nullptr;
	ComPtr<ID3D12Resource> switchResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> renderingHeap = nullptr; // rendering�p�f�B�X�N���v�^�q�[�v

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
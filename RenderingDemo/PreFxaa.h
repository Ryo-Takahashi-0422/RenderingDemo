#pragma once

class PreFxaa
{
    // ������
    void Init();

    // ���[�g�V�O�l�`���̐���
    HRESULT CreateRootSignature();
    // �V�F�[�_�[�ݒ�
    HRESULT ShaderCompile();
    //
    void SetInputLayout();
    /* std::vector<*/D3D12_INPUT_ELEMENT_DESC/*>*/ inputLayout[2];
    // �p�C�v���C���̐���
    HRESULT CreateGraphicPipeline();

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

    // �f�o�C�X
    ComPtr<ID3D12Device> _dev = nullptr;

    // ���[�g�V�O�l�`���֘A
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[5] = {};
    D3D12_ROOT_PARAMETER rootParam[5] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
    ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
    ComPtr<ID3D10Blob> _vsBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
    ComPtr<ID3D10Blob> _psBlob = nullptr; // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�i�[�p
    ComPtr<ID3DBlob> _errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // �V�F�[�_�[���
    //ComPtr<ID3DBlob> shader;
    // �R���s���[�g�p�p�C�v���C��
    ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    // �q�[�v
    ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> extSrvHeap = nullptr;
    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource = nullptr;

    float width;
    float height;

    Integration* m_integration = nullptr;

public:
    PreFxaa(ID3D12Device* dev, Integration* _integration, int _width, int _height);
    ~PreFxaa();
    //void SetSRHeapAndSRV(ComPtr<ID3D12DescriptorHeap> srvHeap);
    void Execution(ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return srvHeap; };
    ComPtr<ID3D12Resource> GetRenderingResource() { return renderingResource; };
};
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

    void CreateInfoViewAndMapping();

    // �f�o�C�X
    ComPtr<ID3D12Device> _dev = nullptr;

    // ���[�g�V�O�l�`���֘A
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[6] = {};
    D3D12_ROOT_PARAMETER rootParam[6] = {};
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
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // �����_�����O���ʊi�[
    ComPtr<ID3D12DescriptorHeap> externalHeap = nullptr; // integration���\�[�X�i�[�p

    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource = nullptr; // �������_�����O����

    ComPtr<ID3D12Resource> colorResource = nullptr; // �����_�[�^�[�Q�b�g1 �J���[�摜
    ComPtr<ID3D12Resource> normalResource = nullptr; // �����_�[�^�[�Q�b�g2 �@���摜
    ComPtr<ID3D12Resource> imguiResource = nullptr; // �����_�[�^�[�Q�b�g3 imgui�摜
    ComPtr<ID3D12Resource> bluedColorResourse = nullptr; // �O�����\�[�X2 blured color
    ComPtr<ID3D12Resource> depthResourse = nullptr; // �O�����\�[�X3 depth
    ComPtr<ID3D12Resource> infoResourse = nullptr; // �O�����\�[�X3 depth

    float width;
    float height;

    struct Infos
    {
        float size;
        bool isFxaa;
    };
    Infos* infos = nullptr;

public:
    PreFxaa(ID3D12Device* dev, int _width, int _height);
    ~PreFxaa();
    //void SetSRHeapAndSRV(ComPtr<ID3D12DescriptorHeap> srvHeap);
    void Execution(ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    void SetResourse1(ComPtr<ID3D12Resource> _resource);
    void SetResourse2(ComPtr<ID3D12Resource> _resource);
    void SetResourse3(ComPtr<ID3D12Resource> _resource);
    void SetResourse4(ComPtr<ID3D12Resource> _resource);
    void SetResourse5(ComPtr<ID3D12Resource> _resource);
    void SetExternalView();

    ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return srvHeap; };
    ComPtr<ID3D12Resource> GetRenderingResource() { return renderingResource; };
};
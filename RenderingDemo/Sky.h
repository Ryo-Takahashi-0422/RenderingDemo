#pragma once

class Sky
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

    // Frustum�֘A
    void InitFrustumReosources();
    HRESULT CreateFrustumHeap();
    HRESULT CreateFrustumResource();
    void CreateSkyView();
    HRESULT MappingFrustum();


    // �f�o�C�X
    ComPtr<ID3D12Device> _dev;
    // �t�F���X
    ComPtr<ID3D12Fence> fence = nullptr;
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
    // �V�F�[�_�[���
    //ComPtr<ID3DBlob> shader;
    // �R���s���[�g�p�p�C�v���C��
    ComPtr<ID3D12PipelineState> pipelineState;
    // �q�[�v
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> skyHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource;
    ComPtr<ID3D12Resource> frustumResource;
    ComPtr<ID3D12Resource> skyLUTResource;

    float resWidth = 256;
    float resHeight = 256;

    Frustum* m_Frustum = nullptr;

public:
    Sky();
    Sky(ID3D12Device* dev, ID3D12Fence* _fence, ID3D12Resource* _skyLUTRsource);
    ~Sky();
    void SetFrustum(Frustum _frustum);
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    void SetSkyLUTResource();
    ComPtr<ID3D12DescriptorHeap> GetSkyLUTRenderingHeap() { return rtvHeap; };
    ComPtr<ID3D12Resource> GetSkyLUTRenderingResource() { return renderingResource; };
};
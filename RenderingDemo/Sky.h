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
    HRESULT CreateSKyHeap();
    HRESULT CreateSkyResources();
    void CreateSkyView();
    HRESULT MappingSkyData();


    // �f�o�C�X
    ComPtr<ID3D12Device> _dev = nullptr;

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
    // �V�F�[�_�[���
    //ComPtr<ID3DBlob> shader;
    // �R���s���[�g�p�p�C�v���C��
    ComPtr<ID3D12PipelineState> pipelineState = nullptr;
    // �q�[�v
    ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> skyHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;
    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource = nullptr;
    ComPtr<ID3D12Resource> frustumResource = nullptr;
    ComPtr<ID3D12Resource> skyLUTResource = nullptr;
    ComPtr<ID3D12Resource> worldMatrixResource = nullptr;

    float width = 64;
    float height = 64;

    Frustum* m_Frustum = nullptr;
    struct SceneInfo
    {
        XMMATRIX world; // world matrix
        int width;
        int height;
    };
    SceneInfo* scneMatrix = nullptr;

    void RecreatreSource();

public:
    Sky();
    Sky(ID3D12Device* dev, ID3D12Fence* _fence, ID3D12Resource* _skyLUTRsource);
    ~Sky();
    void SetFrustum(Frustum _frustum);
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    void ChangeSkyLUTResourceAndView(ID3D12Resource* _skyLUTRsource); // skyLUT�̉𑜓x�ɕύX������ꍇ�͂��̃��\�[�X������ς��Ă��邽�߁A�Q�Ɛ�̃��\�[�X��V���ɐݒ肵�Ċ���view���ݒ肵����
    void SetSceneInfo(XMMATRIX _world);
    
    void ChangeSceneMatrix(XMMATRIX _world);
    void ChangeSceneResolution(int _width, int _height);
    ComPtr<ID3D12DescriptorHeap> GetSkyLUTRenderingHeap() { return rtvHeap; };
    ComPtr<ID3D12Resource> GetSkyLUTRenderingResource() { return renderingResource; };
};
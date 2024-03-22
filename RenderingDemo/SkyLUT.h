#pragma once

struct SkyLUTBuffer
{
    //https://learn.microsoft.com/ja-jp/windows/win32/dxmath/pg-xnamath-optimizing XMFLOAT3��4byte�A���C�������g�����A����16byte�A���C�������g����Ă���
    XMFLOAT3 eyePos;
    float pad0;
    XMFLOAT3 sunDirection;    
    float stepCnt;
    XMFLOAT3 sunIntensity;
    int width;
    int height;
};

class SkyLUT
{
private:

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
    void CreateRenderingCBVSRV();

    // �֗^�}���̐ݒ�
    void InitParticipatingMedia();
    // �֗^�}���p���\�[�X�̐���
    HRESULT CreateParticipatingResource();
    ComPtr<ID3D12Resource> participatingMediaResource;
    ComPtr<ID3D12Resource> skyLUTBufferResource;
    ComPtr<ID3D12Resource> shadowFactorResource;

    // �֗^�}���p�q�[�v�E�r���[�̐���
    HRESULT CreateParticipatingMediaHeapAndView();
    // �֗^�}���p�萔�̃}�b�s���O
    void MappingParticipatingMedia();
    // �}�b�s���O��
    ParticipatingMedia* m_Media = nullptr;
    SkyLUTBuffer* m_SkyLUT = nullptr;

    // �f�o�C�X
    ComPtr<ID3D12Device> _dev;
    // �t�F���X
    //ComPtr<ID3D12Fence> fence = nullptr;
    // ���[�g�V�O�l�`���֘A
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[4] = {};
    D3D12_ROOT_PARAMETER rootParam[4] = {};
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
    ComPtr<ID3D12DescriptorHeap> cbvsrvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> skyLUTHeap = nullptr;
    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource;
    
    // ����M�p�f�[�^
    void* data;
    //// �R�}���h�A���P�[�^
    //ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    //// �R�}���h���X�g
    //ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width;
    UINT64 height;

    void RecreatreSource();
    bool barrierSW = true; // �V���h�E�t�@�N�^�[�̉𑜓x�ω�����я���̕`�掞�ɂ̂�true

public:
    //SkyLUT();
    SkyLUT(ID3D12Device* dev, ID3D12Resource* _shadowFactorRsource, int _width, int _height);
    ~SkyLUT();
    void SetParticipatingMedia(ParticipatingMedia media);
    void SetSkyLUTBuffer(SkyLUTBuffer buffer);
    void SetShadowFactorResource(ID3D12Resource* _shadowFactorRsource);
    void SetSkyLUTResolution();
    void ChangeSkyLUTResolution(int _width, int _height);
    void SetBarrierSWTrue();
    
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect);

    ComPtr<ID3D12DescriptorHeap> GetSkyLUTRenderingHeap() { return cbvsrvHeap; };
    ComPtr<ID3D12Resource> GetSkyLUTRenderingResource() { return renderingResource; };
};
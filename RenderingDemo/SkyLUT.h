#pragma once

struct SkyLUTBuffer
{
    XMFLOAT3 eyePos;
    XMFLOAT3 sunDirection; // 
    float stepCnt;
    XMFLOAT3 sunIntensity;
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
    void CreateRenderingSRV();

    // �֗^�}���̐ݒ�
    void InitParticipatingMedia();
    // �֗^�}���p���\�[�X�̐���
    HRESULT CreateParticipatingResource();
    ComPtr<ID3D12Resource> participatingMediaResource;
    ComPtr<ID3D12Resource> skyLUTBufferResource;
    // �֗^�}���p�q�[�v�E�r���[�̐���
    HRESULT CreateParticipatingMediaHeapAndView();
    // �֗^�}���p�萔�̃}�b�s���O
    void MappingParticipatingMedia();
    // �}�b�s���O��
    ParticipatingMedia m_Media;
    SkyLUTBuffer m_SkyLUT;
    // �R�}���h�̐���
    HRESULT CreateCommand();

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
    // �V�F�[�_�[���
    ComPtr<ID3DBlob> shader;
    // �R���s���[�g�p�p�C�v���C��
    ComPtr<ID3D12PipelineState> pipelineState;
    // �q�[�v
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> skyLUTHeap;
    // ���\�[�X
    ComPtr<ID3D12Resource> renderingResource;
    
    // ����M�p�f�[�^
    void* data;
    // �R�}���h�A���P�[�^
    ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    // �R�}���h���X�g
    ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width = 64;
    UINT64 height = 64;

public:
    SkyLUT();
    SkyLUT(ID3D12Device* dev);
    ~SkyLUT();
    void SetParticipatingMedia(ParticipatingMedia media);
    void SetSkyLUTBuffer(SkyLUTBuffer buffer);

    // ���s
    void Execution(ID3D12CommandQueue* cmdQueue);

};
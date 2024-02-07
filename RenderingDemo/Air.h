#pragma once

class Air
{
private:
    // �𑜓x�֘A
    int width = 1024;
    int height = 1024;
    int depth = 64;
    int threadIdNum_X = 16;
    int threadIdNum_Y = 16;

    // ���[�g�V�O�l�`���̐���
    HRESULT CreateRootSignature();
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[6] = {};
    D3D12_ROOT_PARAMETER rootParam[6] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
    ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
    // �V�F�[�_�[�ݒ�
    HRESULT ShaderCompile();
    // �p�C�v���C���̐���
    HRESULT CreatePipeline();
    // �q�[�v�̐���
    HRESULT CreateHeap();
    // �֗^�}�����\�[�X�̐���
    HRESULT CreateResource();
    // �o�͗p�e�N�X�`�����\�[�X�̐���
    HRESULT CreateTextureResource();
    // �r���[�̐���
    void CreateView();
    // ���\�[�X�̃}�b�v
    HRESULT Mapping();
    // ������
    void Init();
    // �R�}���h�̐���
    //HRESULT CreateCommand();
    // �֗^�}���}�b�s���O��
    ParticipatingMedia* m_Media = nullptr;

    // �f�o�C�X
    ID3D12Device* dev = nullptr;
    // �t�F���X
    ComPtr<ID3D12Fence> fence = nullptr;
    // �R���s���[�g�p���[�g�V�O�l�`��
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    // �V�F�[�_�[���
    ComPtr<ID3D10Blob> csBlob = nullptr; // ���_�V�F�[�_�[�I�u�W�F�N�g�i�[�p
    // �R���s���[�g�p�p�C�v���C��
    ID3D12PipelineState* pipeLine = nullptr;
    // �q�[�v
    ID3D12DescriptorHeap* heap = nullptr;
    // ���\�[�X
    ComPtr<ID3D12Resource> participatingMediaResource = nullptr;
    ComPtr<ID3D12Resource> frustumResource = nullptr;
    ComPtr<ID3D12Resource> sceneInfoResource = nullptr;
    ComPtr<ID3D12Resource> outputTextureResource = nullptr;
    ComPtr<ID3D12Resource> copyTextureResource = nullptr;
    ComPtr<ID3D12Resource> shadowMapResource = nullptr;
    ComPtr<ID3D12Resource> shadowFactorResource = nullptr;
    // �R�}���h�A���P�[�^
    ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
    // �R�}���h���X�g
    ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;

    Frustum* m_Frustum = nullptr;
    struct SceneInfo
    {
        XMMATRIX world;
        XMMATRIX sunViewMatrix;
        XMMATRIX sunProjMatrix;
        XMFLOAT3 eyePos;
        float depthLength;
        XMFLOAT3 adjustedEyePos;
        float distanceLimit;
        XMFLOAT3 sunDirection;
    };
    float depthLengthVal = 100;
    float distanceLimitValue = 1000;

    SceneInfo* m_SceneInfo = nullptr;

public:
    Air(ID3D12Device* dev, ID3D12Fence* _fence, ComPtr<ID3D12Resource> _shadowMapRsource, ComPtr<ID3D12Resource> _shadowFactorRsource);
    ~Air();
    
    ComPtr<ID3D12Resource> GetAirTextureResource() { return copyTextureResource; };
    // ���s
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList);
    void ChangeResolution(int _width, int _height);
    void SetParticipatingMedia(ParticipatingMedia media);
    void SetFrustum(Frustum _frustum);
    void SetSceneInfo(XMMATRIX _sunViewMatrix, XMMATRIX _sunProjMatrix, XMFLOAT3 _eyePos, XMFLOAT3 sunDirection);
};
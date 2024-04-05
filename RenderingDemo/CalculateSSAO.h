#pragma once

class CalculateSSAO
{
private:
    // �𑜓x�֘A
    int width;
    int height;
    int threadIdNum_X = 16;
    int threadIdNum_Y = 16;

    // ���[�g�V�O�l�`���̐���
    HRESULT CreateRootSignature();
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[4] = {};
    D3D12_ROOT_PARAMETER rootParam[4] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
    ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
    // �V�F�[�_�[�ݒ�
    HRESULT ShaderCompile();
    // �p�C�v���C���̐���
    HRESULT CreatePipeline();
    // �q�[�v�̐���
    HRESULT CreateHeap();
    // �o�͗p�e�N�X�`�����\�[�X�̐���
    HRESULT CreateTextureResource();
    // �r���[�̐���
    void CreateView();
    void Mapping();

    // ������
    void Init();

    // �f�o�C�X
    ID3D12Device* _dev = nullptr;
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
    ComPtr<ID3D12Resource> matrixResource = nullptr;
    ComPtr<ID3D12Resource> normalmapResource = nullptr;
    ComPtr<ID3D12Resource> depthmapResource = nullptr;
    ComPtr<ID3D12Resource> outputTextureResource = nullptr;
    ComPtr<ID3D12Resource> copyTextureResource = nullptr;
    // �R�}���h�A���P�[�^
    ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
    // �R�}���h���X�g
    ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;

    struct Matrix4Cal 
    {
        XMMATRIX view;
        //XMMATRIX invView;
        XMMATRIX proj;
        XMMATRIX invProj;
        bool isDraw;

    };
    Matrix4Cal* matrix4Cal = nullptr;

public:
    CalculateSSAO(ID3D12Device* dev, ComPtr<ID3D12Resource> _normalmapResource, ComPtr<ID3D12Resource> _depthmapResource, int _width, int _height);
    ~CalculateSSAO();
    ComPtr<ID3D12Resource> GetTextureResource() { return copyTextureResource; };
    void SetInvVPMatrix(/*XMMATRIX _view, XMMATRIX _invView, */XMMATRIX _proj, XMMATRIX _invProj);
    void SetViewMatrix(XMMATRIX _view);
    // ���s
    void Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList);
    void ChangeResolution(int _width, int _height);
    void SetDraw(bool _isDraw);
    std::pair<float, float> GetResolution() { return std::pair<float, float>(width, height); };
};
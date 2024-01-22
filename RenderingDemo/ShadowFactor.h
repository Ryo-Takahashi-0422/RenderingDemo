#pragma once

class ShadowFactor
{
private:
    // ���[�g�V�O�l�`���̐���
    HRESULT CreateRootSignature();
    CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
    CD3DX12_DESCRIPTOR_RANGE descTableRange[2] = {};
    D3D12_ROOT_PARAMETER rootParam[2] = {};
    ComPtr<ID3DBlob> rootSigBlob = nullptr; // ���[�g�V�O�l�`���I�u�W�F�N�g�i�[�p
    ComPtr<ID3DBlob> errorBlob = nullptr; // �V�F�[�_�[�֘A�G���[�i�[�p
    // �V�F�[�_�[�ݒ�
    HRESULT ShaderCompile();
    // �p�C�v���C���̐���
    HRESULT CreatePipeline();
    // �q�[�v�̐���
    HRESULT CreateHeap();
    // �֗^�}�����\�[�X�̐���
    HRESULT CreateParticipatingMediaResource();
    // �o�͗p�e�N�X�`�����\�[�X�̐���
    HRESULT CreateOutputTextureResource();
    // �r���[�̐���
    void CreateView();
    // ���\�[�X�̃}�b�v
    HRESULT Mapping();
    // ������
    void Init();
    // �R�}���h�̐���
    HRESULT CreateCommand();
    // �֗^�}���}�b�s���O��
    ParticipatingMedia m_Media;

    // �f�o�C�X
    ID3D12Device* _dev = nullptr;
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
    ComPtr<ID3D12Resource> outputTextureResource = nullptr;
    // ����M�p�f�[�^
    void* data = nullptr;
    // �R�}���h�A���P�[�^
    ID3D12CommandAllocator* _cmdAllocator = nullptr;
    // �R�}���h���X�g
    ID3D12GraphicsCommandList* _cmdList = nullptr;

    ComPtr<ID3D11Texture2D> texture = nullptr;

public:
    ShadowFactor(ID3D12Device* dev);
    ~ShadowFactor();
    void SetParticipatingMedia(ParticipatingMedia media);

    // ���s
    void Execution(ID3D12CommandQueue* cmdQueue);

};
#pragma once

class Transmittance
{
private:
    // ���[�g�V�O�l�`���̐���
    HRESULT CreateRoot();
    // �p�C�v���C���̐���
    HRESULT CreatePipe();
    // �q�[�v�̐���
    HRESULT CreateHeap();
    // ���\�[�X�̐���
    HRESULT CreateRsc();
    // UAV�̐���
    void CreateUAV();
    // ���\�[�X�̃}�b�v
    HRESULT Map();
    // ������
    void Init();
    // �R�}���h�̐���
    HRESULT CreateCommand();


    // �f�o�C�X
    ID3D12Device* _dev;
    // �R���s���[�g�p���[�g�V�O�l�`��
    ID3D12RootSignature* root;
    // �V�F�[�_�[���
    ID3DBlob* shader;
    // �R���s���[�g�p�p�C�v���C��
    ID3D12PipelineState* pipe;
    // �q�[�v
    ID3D12DescriptorHeap* heap;
    // ���\�[�X
    ID3D12Resource* rsc;
    // ����M�p�f�[�^
    void* data;
    // �R�}���h�A���P�[�^
    ID3D12CommandAllocator* _cmdAllocator;
    // �R�}���h���X�g
    ID3D12GraphicsCommandList* _cmdList;

public:
    Transmittance(ID3D12Device* dev);
    ~Transmittance();

    // ���s
    void Execution(ID3D12CommandQueue* cmdQueue);

};
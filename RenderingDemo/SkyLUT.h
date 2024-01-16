#pragma once

class SkyLUT
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
    // RTV�̐���
    void CreateRTV();
    // SRV�̐���
    void CreateSRV();
    // ���\�[�X�̃}�b�v
    HRESULT Map();
    // ������
    void Init();
    // �R�}���h�̐���
    HRESULT CreateCommand();


    // �f�o�C�X
    ComPtr<ID3D12Device> _dev;
    // �R���s���[�g�p���[�g�V�O�l�`��
    ComPtr<ID3D12RootSignature> root;
    // �V�F�[�_�[���
    ComPtr<ID3DBlob> shader;
    // �R���s���[�g�p�p�C�v���C��
    ComPtr<ID3D12PipelineState> pipe;
    // �q�[�v
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    // ���\�[�X
    ComPtr<ID3D12Resource> rsc;
    // ����M�p�f�[�^
    void* data;
    // �R�}���h�A���P�[�^
    ComPtr<ID3D12CommandAllocator> _cmdAllocator;
    // �R�}���h���X�g
    ComPtr<ID3D12GraphicsCommandList> _cmdList;

    UINT64 width = 64;
    UINT64 height = 64;

public:
    SkyLUT(ID3D12Device* dev);
    ~SkyLUT();

    // ���s
    void Execution(ID3D12CommandQueue* cmdQueue);

};
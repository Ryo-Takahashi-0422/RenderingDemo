#include <stdafx.h>
#include <SkyLUT.h>


SkyLUT::SkyLUT(ID3D12Device* _dev) :
    _dev(_dev), root(nullptr), shader(nullptr), pipe(nullptr), srvHeap(nullptr), rsc(nullptr),
    data(nullptr), _cmdAllocator(nullptr), _cmdList(nullptr)
{
    Init();
    CreateCommand();
}
SkyLUT::~SkyLUT()
{
    D3D12_RANGE range{ 0, 1 };
    rsc->Unmap(0, &range);
}

// ���[�g�V�O�l�`���̐���
HRESULT SkyLUT::CreateRoot()
{
    ////�V�F�[�_�[�R���p�C��
    //auto hr = D3DCompileFromFile(L"Test.hlsl", nullptr,
    //    D3D_COMPILE_STANDARD_FILE_INCLUDE, "CS", "cs_5_1", D3DCOMPILE_DEBUG |
    //    D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shader, nullptr);
    ////�V�F�[�_�[���烋�[�g�V�O�l�`�������擾
    //ID3DBlob* sig = nullptr;
    //hr = D3DGetBlobPart(shader->GetBufferPointer(), shader->GetBufferSize(),
    //    D3D_BLOB_ROOT_SIGNATURE, 0, &sig);
    ////���[�g�V�O�l�`���̐���
    //hr = _dev->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(),
    //    IID_PPV_ARGS(&root));
    //return hr;
}

// �p�C�v���C���̐���
HRESULT SkyLUT::CreatePipe()
{
    //D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    //desc.CS.pShaderBytecode = shader->GetBufferPointer();
    //desc.CS.BytecodeLength = shader->GetBufferSize();
    //desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    //desc.NodeMask = 0;
    //desc.pRootSignature = root.Get();

    //auto hr = _dev->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipe));
    //return hr;
}

// �q�[�v�̐���
HRESULT SkyLUT::CreateHeap()
{
    //����̓e�X�g�Ƃ��ă��\�[�X��1�Ƃ��č쐬
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto hr = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap));
    return hr;
}

// ���\�[�X�̐���
HRESULT SkyLUT::CreateRsc()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	auto depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = width;
	resDesc.Height = height;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	auto result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(rsc.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;
}

// RTV�̍쐬
void SkyLUT::CreateRTV()
{
    // create RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV�p�f�B�X�N���v�^�q�[�v
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2; // ������
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
    if (result != S_OK) return;

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _dev->CreateRenderTargetView//���\�[�X�f�[�^(_backBuffers)�ɃA�N�Z�X���邽�߂̃����_�[�^�[�Q�b�g�r���[��handle�A�h���X�ɍ쐬
    (
        rsc.Get(),//�����_�[�^�[�Q�b�g��\�� ID3D12Resource �I�u�W�F�N�g�ւ̃|�C���^�[
        &rtvDesc,//�����_�[ �^�[�Q�b�g �r���[���L�q���� D3D12_RENDER_TARGET_VIEW_DESC �\���̂ւ̃|�C���^�[�B
        handle//�V�����쐬���ꂽ�����_�[�^�[�Q�b�g�r���[�����݂��鈶���\�� CPU �L�q�q�n���h��(�q�[�v��̃A�h���X)
    );
}

// SRV�̐���
void SkyLUT::CreateSRV()
{
    auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        rsc.Get(),
        &srvDesc,
        handle
    );
}

// ���\�[�X�̃}�b�v
HRESULT SkyLUT::Map()
{
    D3D12_RANGE range{ 0, 1 };
    auto hr = rsc->Map(0, &range, &data);
    return hr;
}

// ������
void SkyLUT::Init()
{
    CreateRoot();
    CreatePipe();
    CreateHeap();
    CreateRsc();
    CreateRTV();
    CreateSRV();
    Map();
}

// �R�}���h�̐���
HRESULT SkyLUT::CreateCommand()
{
    auto hr = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
        IID_PPV_ARGS(&_cmdAllocator));
    hr = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, _cmdAllocator.Get(), nullptr,
        IID_PPV_ARGS(&_cmdList));
    return hr;
}

// ���s
void SkyLUT::Execution(ID3D12CommandQueue* queue)
{
    //�R�}���h�̃��Z�b�g
    _cmdAllocator->Reset();
    _cmdList->Reset(_cmdAllocator.Get(), nullptr);
    //���ꂼ��̃Z�b�g
    _cmdList->SetComputeRootSignature(root.Get());
    _cmdList->SetPipelineState(pipe.Get());
    _cmdList->SetDescriptorHeaps(1, &srvHeap);

    auto handle = srvHeap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetComputeRootDescriptorTable(0, handle);

    //�R���s���[�g�V�F�[�_�[�̎��s(�����256�̃X���b�h�O���[�v���w��)
    //_cmdList->Dispatch(test.size(), 1, 1);

    _cmdList->Close();

    //�R�}���h�̎��s
    ID3D12CommandList* executeList[] = { _cmdList.Get() };
    queue->ExecuteCommandLists(1, executeList);


    //-----������ID3D12Fence�̑ҋ@��������-----

}
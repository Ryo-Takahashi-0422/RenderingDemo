#include <stdafx.h>
#include <ShadowFactor.h>

// �m�F�p�z��
std::vector<float>test(256, 0);

ShadowFactor::ShadowFactor(ID3D12Device* _dev) : _dev(_dev)
{
    Init();
    CreateCommand();
}
ShadowFactor::~ShadowFactor()
{
    D3D12_RANGE range{ 0, 1 };
    participatingMediaResource->Unmap(0, &range);
}

// ������
void ShadowFactor::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreatePipeline();
    CreateHeap();
    CreateParticipatingMediaResource();
    CreateOutputTextureResource();
    CreateView();
    Mapping();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT ShadowFactor::CreateRootSignature()
{
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // martix
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // martix

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
    rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 2;
    rootSignatureDesc.pParameters = rootParam;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = stSamplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    auto result = D3D12SerializeRootSignature //�V���A����
    (
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        rootSigBlob.ReleaseAndGetAddressOf(),
        errorBlob.GetAddressOf()
    );

    result = _dev->CreateRootSignature
    (
        0,
        rootSigBlob->GetBufferPointer(),
        rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())
    );

    rootSigBlob->Release();

    return result;
}

// �V�F�[�_�[�ݒ�
HRESULT ShadowFactor::ShaderCompile()
{
    //�V�F�[�_�[�R���p�C��
    auto result = D3DCompileFromFile
    (
        L"ShadowFactor.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "cs_main",
        "cs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        csBlob.ReleaseAndGetAddressOf(),
        errorBlob.GetAddressOf()
    );

    //�G���[�`�F�b�N
    if (FAILED(result))
    {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            ::OutputDebugStringA("�t�@�C����������܂���");
            csBlob = nullptr;
            return result;
        }
        else
        {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());

            std::copy_n((char*)errorBlob->GetBufferPointer(),
                errorBlob->GetBufferSize(),
                errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
    }

    return result;
}

// �p�C�v���C���̐���
HRESULT ShadowFactor::CreatePipeline()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.CS.pShaderBytecode = csBlob->GetBufferPointer();
    desc.CS.BytecodeLength = csBlob->GetBufferSize();
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask = 0;
    desc.pRootSignature = rootSignature.Get();

    auto result = _dev->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeLine));
    return result;
}

// �q�[�v�̐���
HRESULT ShadowFactor::CreateHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 3;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    return result;
}

// �֗^�}�����\�[�X�̐���
HRESULT ShadowFactor::CreateParticipatingMediaResource()
{
    // ParticipatingMedia�p
    auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ParticipatingMedia) + 0xff) & ~0xff);

    auto result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(participatingMediaResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;
}

// �o�͗p�e�N�X�`�����\�[�X�̐���
HRESULT ShadowFactor::CreateOutputTextureResource()
{
    CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    //�T�C�Y�͒萔�o�b�t�@�Ɠ����悤�Ɏw��
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.Width = 256;
    textureDesc.Height = 256;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    auto result = _dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(&outputTextureResource));
    return result;
}

// �r���[�̐���
void ShadowFactor::CreateView()
{   
    auto handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // ParticipatingMedia�pview
    D3D12_CONSTANT_BUFFER_VIEW_DESC pMediaCbvDesc = {};
    pMediaCbvDesc.BufferLocation = participatingMediaResource->GetGPUVirtualAddress();
    pMediaCbvDesc.SizeInBytes = participatingMediaResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &pMediaCbvDesc,
        handle
    );

    handle.ptr += inc;
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;

    _dev->CreateUnorderedAccessView
    (
        outputTextureResource.Get(),
        nullptr,
        &desc,
        handle
    );

    // ��UAV���e�N�X�`���Ƃ���Sample���邽�߂�SRV�����K�v����
}

// ���\�[�X�̃}�b�v
HRESULT ShadowFactor::Mapping()
{    
    // ParticipatingMedia
    auto result = participatingMediaResource->Map(0, nullptr, (void**)&m_Media);
    return result;
}

// �R�}���h�̐���
HRESULT ShadowFactor::CreateCommand()
{
    auto result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
        IID_PPV_ARGS(&_cmdAllocator));
    result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, _cmdAllocator, nullptr,
        IID_PPV_ARGS(&_cmdList));
    return result;
}

// ���s
void ShadowFactor::Execution(ID3D12CommandQueue* queue)
{
    //�R�}���h�̃��Z�b�g
    _cmdAllocator->Reset();
    _cmdList->Reset(_cmdAllocator, nullptr);
    //���ꂼ��̃Z�b�g
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipeLine);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetComputeRootDescriptorTable(0, handle);

    //�R���s���[�g�V�F�[�_�[�̎��s(�����256�̃X���b�h�O���[�v���w��)
    _cmdList->Dispatch(test.size(), 1, 1);

    _cmdList->Close();

    //�R�}���h�̎��s
    ID3D12CommandList* executeList[] = { _cmdList };
    queue->ExecuteCommandLists(1, executeList);

    
    //-----������ID3D12Fence�̑ҋ@��������-----
    

    //GPU����f�[�^�����炤
    test.assign((float*)data, (float*)data + test.size());
 }

// �O������̊֗^�}���ݒ�
void ShadowFactor::SetParticipatingMedia(ParticipatingMedia media)
{
    m_Media = media;
}

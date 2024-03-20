#include <stdafx.h>
#include <ShadowFactor.h>

ShadowFactor::ShadowFactor(ID3D12Device* _dev, ID3D12Fence* _fence) : _dev(_dev), fence(_fence)
{
    Init();
    //CreateCommand();
}
ShadowFactor::~ShadowFactor()
{
    // ComPtr�����N���X��ComPtr�Ƃ��Ĉ���Ȃ��ƈȉ��̂悤�Ƀf�X�g���N�^�Ή����K�v�ɂȂ�ʓ|...
    _dev->Release();
    _dev = nullptr;
    pipeLine->Release();
    pipeLine = nullptr;
    heap->Release();
    heap = nullptr;
    participatingMediaResource->Unmap(0, nullptr);
    m_Media = nullptr;
}

// ������
void ShadowFactor::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreatePipeline();
    CreateHeap();
    CreateParticipatingMediaResource();
    CreateTextureResource();
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

    //rootSigBlob->Release();

    return result;
}

// �V�F�[�_�[�ݒ�
HRESULT ShadowFactor::ShaderCompile()
{
    std::string cs = "ShadowFactor.hlsl";
    std::string noUse = "nothing";
    auto pair = Utility::GetHlslFilepath(cs, noUse);

    //�V�F�[�_�[�R���p�C��
    auto result = D3DCompileFromFile
    (
        pair.first,
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
    desc.NumDescriptors = 3; // CBV, UAV, SRV
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
HRESULT ShadowFactor::CreateTextureResource()
{
    CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    //Compute Shader�o�͗p
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    
    auto result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&outputTextureResource)
    );
    if (result != S_OK) return result;

    // Compute Shader���o�͂����e�N�X�`�����R�s�[���āASRV�Ƃ��Ĉ������߂̃��\�[�X
    //textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&copyTextureResource)
    );

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

    handle.ptr += inc;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        copyTextureResource.Get(),
        &srvDesc,
        handle
    );
}

// ���\�[�X�̃}�b�v
HRESULT ShadowFactor::Mapping()
{    
    // ParticipatingMedia
    auto result = participatingMediaResource->Map(0, nullptr, (void**)&m_Media);
    return result;
}

// ���s
void ShadowFactor::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList)
{
    //���ꂼ��̃Z�b�g
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipeLine);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetComputeRootDescriptorTable(0, handle);

    handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetComputeRootDescriptorTable(1, handle);

    // �摜�𑜓x��1�O���[�v������̗v�f���Ŋ��邱�ƂŁA�O���[�v�����ǂꂾ���K�v���Z�o����B���̌��ʂ�����؂�Ȃ��ꍇ�̂��Ƃ𓥂܂��ATHREAD_GROUP_SIZE_X - 1) / THREAD_GROUP_SIZE_X�𑫂��Ă���B
    // ��Fres.x=415�ATHREAD_GROUP_SIZE_X = 16�̎��A25.9375�B�����15/16=0.9375�𑫂���int 26����
    int threadGroupNum_X = (width + threadIdNum_X - 1) / threadIdNum_X;
    int threadGroupNum_Y = (height + threadIdNum_Y - 1) / threadIdNum_Y;
    _cmdList->Dispatch(threadGroupNum_X, threadGroupNum_Y, 1);

    // �o�͗p���\�[�X��Ԃ��R�s�[����Ԃɐݒ�
    auto barrierDescOfOutputTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        outputTextureResource.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_SOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfOutputTexture);

    // �R�s�[�p���\�[�X��Ԃ��R�s�[���Ԃɐݒ�
    auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        copyTextureResource.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_DEST
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

    // �`�悵���e�N�X�`�����R�s�[�@�`�挳��UAV�Ȃ̂�SRV�Ƃ��ėp�ӂ������\�[�X�ɃR�s�[���邱�ƂŃe�N�X�`���Ƃ��ė��p�o����悤�ɂȂ�B
    _cmdList->CopyResource(copyTextureResource.Get(), outputTextureResource.Get());

    // �o�͗p���\�[�X��Ԃ����̏�Ԃɖ߂�
    barrierDescOfOutputTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        outputTextureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfOutputTexture);

    // �R�s�[�p���\�[�X��Ԃ�SkyLUT.hlsl�œǂݍ��߂��Ԃɂ���
    barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        copyTextureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);
 }

// �O������̊֗^�}���ݒ�
void ShadowFactor::SetParticipatingMedia(ParticipatingMedia media)
{
    m_Media->rayleighScattering = media.rayleighScattering;
    m_Media->mieScattering = media.mieScattering;
    m_Media->mieAbsorption = media.mieAbsorption;
    m_Media->ozoneAbsorption = media.ozoneAbsorption;
    m_Media->asymmetryParameter = media.asymmetryParameter;
    m_Media->altitudeOfRayleigh = media.altitudeOfRayleigh;
    m_Media->altitudeOfMie = media.altitudeOfMie;
    m_Media->halfWidthOfOzone = media.halfWidthOfOzone;
    m_Media->altitudeOfOzone = media.altitudeOfOzone;
    m_Media->groundRadius = media.groundRadius;
    m_Media->atomosphereRadius = media.atomosphereRadius;
}

void ShadowFactor::ChangeResolution(int _width, int _height)
{
    width = _width;
    height = _height;

    outputTextureResource->Release();
    outputTextureResource = nullptr;
    copyTextureResource->Release();
    copyTextureResource = nullptr;
    heap->Release();
    heap = nullptr;

    CreateHeap();
    CreateTextureResource();
    CreateView();
}
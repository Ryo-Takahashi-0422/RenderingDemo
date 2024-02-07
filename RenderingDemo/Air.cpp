#include <stdafx.h>
#include <Air.h>

Air::Air(ID3D12Device* _dev, ID3D12Fence* _fence, ComPtr<ID3D12Resource> _shadowMapRsource, ComPtr<ID3D12Resource> _shadowFactorRsource) : dev(_dev), fence(_fence), shadowMapResource(_shadowMapRsource), shadowFactorResource(_shadowFactorRsource)
{
    Init();
    //CreateCommand();
}
Air::~Air()
{
    //D3D12_RANGE range{ 0, 1 };
    //participatingMediaResource->Unmap(0, &range);
}

// ������
void Air::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreatePipeline();
    CreateHeap();
    CreateResource();
    CreateTextureResource();
    CreateView();
    Mapping();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT Air::CreateRootSignature()
{
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // participating media
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // �o�͌��ʊi�[�p
    descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // frustum
    descTableRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2); // �e��v�Z�p�f�[�^�Q
    descTableRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // shadowMap
    descTableRange[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // shadowFactor

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
    rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[2].DescriptorTable.pDescriptorRanges = &descTableRange[2];
    rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[3].DescriptorTable.pDescriptorRanges = &descTableRange[3];
    rootParam[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[4].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[4].DescriptorTable.pDescriptorRanges = &descTableRange[4];
    rootParam[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootParam[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[5].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[5].DescriptorTable.pDescriptorRanges = &descTableRange[5];
    rootParam[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 6;
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

    result = dev->CreateRootSignature
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
HRESULT Air::ShaderCompile()
{
    std::string cs = "Air.hlsl";
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
HRESULT Air::CreatePipeline()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.CS.pShaderBytecode = csBlob->GetBufferPointer();
    desc.CS.BytecodeLength = csBlob->GetBufferSize();
    desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    desc.NodeMask = 0;
    desc.pRootSignature = rootSignature.Get();

    auto result = dev->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeLine));
    return result;
}

// �q�[�v�̐���
HRESULT Air::CreateHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 6; // CBV(ParticipatingMedia), CBV(Frustum), CBV(SceneInfo) UAV, SRV(DepthMap from Sun), SRV(ShadowFactor)
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    return result;
}

// �e��v�Z�p���\�[�X�̐���
HRESULT Air::CreateResource()
{
    // ParticipatingMedia�p
    auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ParticipatingMedia) + 0xff) & ~0xff);

    auto result = dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(participatingMediaResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    // Frustum�p
    desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(Frustum) + 0xff) & ~0xff);

    result = dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(frustumResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;

    // SceneInfo�p
    desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneInfo) + 0xff) & ~0xff);

    result = dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(sceneInfoResource.ReleaseAndGetAddressOf())
    );

    return result;
}

// �o�͗p�e�N�X�`�����\�[�X�̐���
HRESULT Air::CreateTextureResource()
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

    auto result = dev->CreateCommittedResource
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
    result = dev->CreateCommittedResource
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
void Air::CreateView()
{
    auto handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto inc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // ParticipatingMedia�pview
    D3D12_CONSTANT_BUFFER_VIEW_DESC pMediaCbvDesc = {};
    pMediaCbvDesc.BufferLocation = participatingMediaResource->GetGPUVirtualAddress();
    pMediaCbvDesc.SizeInBytes = participatingMediaResource->GetDesc().Width;
    dev->CreateConstantBufferView
    (
        &pMediaCbvDesc,
        handle
    );

    // frustum�pview
    handle.ptr += inc;
    D3D12_CONSTANT_BUFFER_VIEW_DESC frustumCbvDesc = {};
    frustumCbvDesc.BufferLocation = frustumResource->GetGPUVirtualAddress();
    frustumCbvDesc.SizeInBytes = frustumResource->GetDesc().Width;
    dev->CreateConstantBufferView
    (
        &frustumCbvDesc,
        handle
    );

    // SceneInfo�pview
    handle.ptr += inc;
    D3D12_CONSTANT_BUFFER_VIEW_DESC sceneInfoCbvDesc = {};
    sceneInfoCbvDesc.BufferLocation = sceneInfoResource->GetGPUVirtualAddress();
    sceneInfoCbvDesc.SizeInBytes = sceneInfoResource->GetDesc().Width;
    dev->CreateConstantBufferView
    (
        &sceneInfoCbvDesc,
        handle
    );

    // �o�͗pUAV
    handle.ptr += inc;
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;

    dev->CreateUnorderedAccessView
    (
        outputTextureResource.Get(),
        nullptr,
        &desc,
        handle
    );

    // �R�s�[�pUAV
    handle.ptr += inc;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    dev->CreateShaderResourceView
    (
        copyTextureResource.Get(),
        &srvDesc,
        handle
    );
}

// ���\�[�X�̃}�b�v
HRESULT Air::Mapping()
{
    // ParticipatingMedia
    auto result = participatingMediaResource->Map(0, nullptr, (void**)&m_Media);
    if (result != S_OK) return result;

    // Frustum
    result = frustumResource->Map(0, nullptr, (void**)&m_Frustum);
    if (result != S_OK) return result;

    // sceneinfo
    result = sceneInfoResource->Map(0, nullptr, (void**)&m_SceneInfo);
    return result;
}

// ���s
void Air::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList)
{
    //���ꂼ��̃Z�b�g
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipeLine);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    auto inc = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetComputeRootDescriptorTable(0, handle); // participating media

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(1, handle); // uav output 3d texture

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(2, handle); // frustum

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(3, handle); // sceneinfo

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(4, handle); // depthmap from sun

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(5, handle); // shadowfactor

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
void Air::SetParticipatingMedia(ParticipatingMedia media)
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

void Air::ChangeResolution(int _width, int _height)
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

// �O�������Frustum�ݒ�
void Air::SetFrustum(Frustum _frustum)
{
    m_Frustum->topLeft = _frustum.topLeft;
    m_Frustum->topRight = _frustum.topRight;
    m_Frustum->bottomLeft = _frustum.bottomLeft;
    m_Frustum->bottomRight = _frustum.bottomRight;
}

// �O�������SceneInfo�ݒ�
void Air::SetSceneInfo(XMMATRIX _world)
{
    m_SceneInfo->world;
    m_SceneInfo->sunViewMatrix;
    m_SceneInfo->sunProjMatrix;
    m_SceneInfo->eyePos;
    m_SceneInfo->tDistance;
    m_SceneInfo->adjustedEyePos;
    m_SceneInfo->limitDistance;
    m_SceneInfo->sunDirection;
}
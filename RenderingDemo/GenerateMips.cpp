#include <stdafx.h>
#include <GenerateMips.h>

GenerateMips::GenerateMips(ID3D12Device* _dev, int _width, int _height) : _dev(_dev)
{
    width = _width;
    height = _height;
    Init();
}
GenerateMips::~GenerateMips()
{
    _dev->Release();
    _dev = nullptr;
    pipeLine->Release();
    pipeLine = nullptr;
    heap->Release();
    heap = nullptr;
}

// ������
void GenerateMips::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreatePipeline();
    CreateHeap();
    CreateTextureResource();
    CreateView();
    //Mapping();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT GenerateMips::CreateRootSignature()
{
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // output1
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1); // output2
    descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2); // output3
    descTableRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3); // output4
    descTableRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // src

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

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 5;
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
HRESULT GenerateMips::ShaderCompile()
{
    std::string cs = "GenerateMipsCS.hlsl";
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
HRESULT GenerateMips::CreatePipeline()
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
HRESULT GenerateMips::CreateHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 6; // output1-4, src, copy dst
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    auto result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    return result;
}

// �o�͗p�e�N�X�`�����\�[�X�̐���
HRESULT GenerateMips::CreateTextureResource()
{
    CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    //Compute Shader�o�͗p
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    //auto result = _dev->CreateCommittedResource
    //(
    //    &prop,
    //    D3D12_HEAP_FLAG_NONE,
    //    &textureDesc,
    //    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    //    nullptr,
    //    IID_PPV_ARGS(&outputTextureResource1)
    //);
    //if (result != S_OK) return result;

    //result = _dev->CreateCommittedResource
    //(
    //    &prop,
    //    D3D12_HEAP_FLAG_NONE,
    //    &textureDesc,
    //    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    //    nullptr,
    //    IID_PPV_ARGS(&outputTextureResource2)
    //);
    //if (result != S_OK) return result;

    //result = _dev->CreateCommittedResource
    //(
    //    &prop,
    //    D3D12_HEAP_FLAG_NONE,
    //    &textureDesc,
    //    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    //    nullptr,
    //    IID_PPV_ARGS(&outputTextureResource3)
    //);
    //if (result != S_OK) return result;

    //result = _dev->CreateCommittedResource
    //(
    //    &prop,
    //    D3D12_HEAP_FLAG_NONE,
    //    &textureDesc,
    //    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    //    nullptr,
    //    IID_PPV_ARGS(&outputTextureResource4)
    //);
    //if (result != S_OK) return result;

    // Compute Shader���o�͂����e�N�X�`�����R�s�[���āASRV�Ƃ��Ĉ������߂̃��\�[�X
    //textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    auto result = _dev->CreateCommittedResource
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
void GenerateMips::CreateView()
{
    auto handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Texture2D.MipSlice = 1;

    _dev->CreateUnorderedAccessView
    (
        srcResource.Get(),
        nullptr,
        &desc,
        handle
    );

    handle.ptr += inc;
    desc.Texture2D.MipSlice += 1;

    _dev->CreateUnorderedAccessView
    (
        srcResource.Get(),
        nullptr,
        &desc,
        handle
    );

    handle.ptr += inc;
    desc.Texture2D.MipSlice += 1;

    _dev->CreateUnorderedAccessView
    (
        srcResource.Get(),
        nullptr,
        &desc,
        handle
    );

    handle.ptr += inc;
    desc.Texture2D.MipSlice += 1;

    _dev->CreateUnorderedAccessView
    (
        srcResource.Get(),
        nullptr,
        &desc,
        handle
    );

    handle.ptr += inc;

    // src
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MipLevels = 5;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        srcResource.Get(),
        &srvDesc,
        handle
    );

    handle.ptr += inc;

    // copy dst
    _dev->CreateShaderResourceView
    (
        copyTextureResource.Get(),
        &srvDesc,
        handle
    );
}

void GenerateMips::SetResource(ComPtr<ID3D12Resource> _src)
{
    srcResource = _src;
}

// ���s
void GenerateMips::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList)
{
    //���ꂼ��̃Z�b�g
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipeLine);
    _cmdList->SetDescriptorHeaps(1, &heap);

    auto handle = heap->GetGPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);;
    _cmdList->SetComputeRootDescriptorTable(0, handle); // output1

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(1, handle); // output2

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(2, handle); // output3

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(3, handle); // output4

    handle.ptr += inc;
    _cmdList->SetComputeRootDescriptorTable(4, handle); // source

    // �摜�𑜓x��1�O���[�v������̗v�f���Ŋ��邱�ƂŁA�O���[�v�����ǂꂾ���K�v���Z�o����B���̌��ʂ�����؂�Ȃ��ꍇ�̂��Ƃ𓥂܂��ATHREAD_GROUP_SIZE_X - 1) / THREAD_GROUP_SIZE_X�𑫂��Ă���B
    // ��Fres.x=415�ATHREAD_GROUP_SIZE_X = 16�̎��A25.9375�B�����15/16=0.9375�𑫂���int 26����
    int threadGroupNum_X = (width + threadIdNum_X - 1) / threadIdNum_X;
    int threadGroupNum_Y = (height + threadIdNum_Y - 1) / threadIdNum_Y;
    _cmdList->Dispatch(threadGroupNum_X, threadGroupNum_Y, 1);

    // �o�͗p���\�[�X��Ԃ��R�s�[����Ԃɐݒ�
    auto barrierDescOfOutputTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        srcResource.Get(),
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
    _cmdList->CopyResource(copyTextureResource.Get(), srcResource.Get());

    // �o�͗p���\�[�X��Ԃ����̏�Ԃɖ߂�
    barrierDescOfOutputTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        srcResource.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfOutputTexture);

    // �R�s�[�p���\�[�X��Ԃ��e�N�X�`���Ƃ��ēǂݍ��߂��Ԃɂ���
    barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
    (
        copyTextureResource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);
}

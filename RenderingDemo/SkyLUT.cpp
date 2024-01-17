#include <stdafx.h>
#include <SkyLUT.h>


SkyLUT::SkyLUT(ID3D12Device* _dev) :
    _dev(_dev), shader(nullptr), pipelineState(nullptr), srvHeap(nullptr), mediaHeap(nullptr), renderingResource(nullptr), participatingMedaiResource(nullptr),
    data(nullptr), _cmdAllocator(nullptr), _cmdList(nullptr)
{
    Init();
    CreateCommand();
}

SkyLUT::~SkyLUT()
{
    D3D12_RANGE range{ 0, 1 };
}

// ������
void SkyLUT::Init()
{
    CreateRootSignature();
    ShaderCompile();
    CreateGraphicPipeline();
    RenderingSet();
    ParticipatingMediaSet();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT SkyLUT::CreateRootSignature()
{    
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // martix

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1; // WVP�p
    rootParam[0].DescriptorTable.pDescriptorRanges = descTableRange;
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 11;
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

//  �V�F�[�_�[�ݒ�
HRESULT SkyLUT::ShaderCompile()
{
    auto result = D3DCompileFromFile
    (
        L"C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\RenderingDemo\\SkyLUT.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        _vsBlob.ReleaseAndGetAddressOf()
        , _errorBlob.GetAddressOf()
    );

    result = D3DCompileFromFile
    (
        L"C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\RenderingDemo\\SkyLUT.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        _psBlob.ReleaseAndGetAddressOf()
        , _errorBlob.GetAddressOf()
    );

    //�G���[�`�F�b�N
    if (FAILED(result))
    {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            ::OutputDebugStringA("�t�@�C����������܂���");
            _vsBlob = nullptr;
            _psBlob = nullptr;
            return result;
        }
        else
        {
            std::string errstr;
            errstr.resize(_errorBlob->GetBufferSize());

            std::copy_n((char*)_errorBlob->GetBufferPointer(),
                _errorBlob->GetBufferSize(),
                errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
    }

    return result;
}

// 
void SkyLUT::SetInputLayout()
{
    // ���W
    inputLayout[0] =
    {        
        "POSITION",
        0, // �����Z�}���e�B�N�X�ɑ΂���C���f�b�N�X
        DXGI_FORMAT_R32G32B32_FLOAT,
        0, // �X���b�g�C���f�b�N�X
        D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0 // ��x�ɕ`�悷��C���X�^���X��
        
    };

    // UV
    inputLayout[1] =
    {
        "TEXCOORD",
        0,
        DXGI_FORMAT_R32G32_FLOAT,
        0, // �X���b�g�C���f�b�N�X
        D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0        
    };
}

// �p�C�v���C���̐���
HRESULT SkyLUT::CreateGraphicPipeline()
{
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetDesc = {};
    renderTargetDesc.BlendEnable = false;//�u�����h��L���ɂ��邩�����ɂ��邩
    renderTargetDesc.LogicOpEnable = false;//�_�������L���ɂ��邩�����ɂ��邩
    renderTargetDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSignature.Get();

    if (_vsBlob != nullptr)
    {
        desc.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
        desc.VS.BytecodeLength = _vsBlob->GetBufferSize();
    }

    if (_psBlob != nullptr)
    {
        desc.PS.pShaderBytecode = _psBlob->GetBufferPointer();
        desc.PS.BytecodeLength = _psBlob->GetBufferSize();
    }

    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    desc.RasterizerState.MultisampleEnable = false;
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.RasterizerState.DepthClipEnable = true;

    desc.BlendState.AlphaToCoverageEnable = false;
    desc.BlendState.IndependentBlendEnable = false;

    desc.BlendState.RenderTarget[0] = renderTargetDesc;
    desc.InputLayout.pInputElementDescs = inputLayout;

    desc.InputLayout.NumElements = _countof(inputLayout);

    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // model

    desc.SampleDesc.Count = 1; //1�T���v��/�s�N�Z��
    desc.SampleDesc.Quality = 0;

    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE/*D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT*/;

    desc.DepthStencilState.DepthEnable = true;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // �[�x�o�b�t�@�[�ɐ[�x�l��`������
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // �\�[�X�f�[�^���R�s�[��f�[�^��菬�����ꍇ��������
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget�ݒ�
void SkyLUT::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
}

// �q�[�v�̐���
HRESULT SkyLUT::CreateRenderingHeap()
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
HRESULT SkyLUT::CreateRenderingResource()
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
		IID_PPV_ARGS(renderingResource.ReleaseAndGetAddressOf())
	);
	if (result != S_OK) return result;
}

// RTV�̍쐬
void SkyLUT::CreateRenderingRTV()
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
        renderingResource.Get(),//�����_�[�^�[�Q�b�g��\�� ID3D12Resource �I�u�W�F�N�g�ւ̃|�C���^�[
        &rtvDesc,//�����_�[ �^�[�Q�b�g �r���[���L�q���� D3D12_RENDER_TARGET_VIEW_DESC �\���̂ւ̃|�C���^�[�B
        handle//�V�����쐬���ꂽ�����_�[�^�[�Q�b�g�r���[�����݂��鈶���\�� CPU �L�q�q�n���h��(�q�[�v��̃A�h���X)
    );
}

// SRV�̐���
void SkyLUT::CreateRenderingSRV()
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
        renderingResource.Get(),
        &srvDesc,
        handle
    );
}


// �O������̊֗^�}���ݒ�
void SkyLUT::SetParticipatingMedia(ParticipatingMedia media)
{
    m_Media = media;
}

// �֗^�}���̐ݒ�
void SkyLUT::ParticipatingMediaSet()
{
    CreateParticipatingResource();
    CreateParticipatingMediaHeap();
    MappingParticipatingMedia();
}

// �֗^�}���p���\�[�X�̐���
HRESULT SkyLUT::CreateParticipatingResource()
{
    auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ParticipatingMedia) + 0xff) & ~0xff);

    auto result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(participatingMedaiResource.ReleaseAndGetAddressOf())
    );
    if (result != S_OK) return result;
}

// �֗^�}���p�q�[�v�̐���
HRESULT SkyLUT::CreateParticipatingMediaHeap()
{
    // create view
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = participatingMedaiResource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = participatingMedaiResource->GetDesc().Width;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV�p�f�B�X�N���v�^�q�[�v
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mediaHeap.GetAddressOf()));
    if (result != S_OK) return result;

    auto handle = mediaHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //  1:Matrix(world, view, proj)

    _dev->CreateConstantBufferView
    (
        &cbvDesc,
        handle
    );
}

// �֗^�}���p�萔�̃}�b�s���O
void SkyLUT::MappingParticipatingMedia()
{
    participatingMedaiResource->Map(0, nullptr, (void**)&m_Media);
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
    _cmdList->SetComputeRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
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
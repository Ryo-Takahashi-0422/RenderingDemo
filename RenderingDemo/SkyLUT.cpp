#include <stdafx.h>
#include <SkyLUT.h>


SkyLUT::SkyLUT(ID3D12Device* _dev, ID3D12Resource* _shadowFactorRsource, int _width, int _height) :
    _dev(_dev), pipelineState(nullptr), renderingResource(nullptr), participatingMediaResource(nullptr), shadowFactorResource(_shadowFactorRsource), data(nullptr)
{
    m_Media = new ParticipatingMedia;
    m_SkyLUT = new SkyLUTBuffer;
    width = _width;
    height = _height;
    Init();
}

SkyLUT::~SkyLUT()
{
    participatingMediaResource->Unmap(0, nullptr);
    m_Media = nullptr;

    skyLUTBufferResource->Unmap(0, nullptr);
    m_SkyLUT = nullptr;
}

// ������
void SkyLUT::Init()
{
    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();
    InitParticipatingMedia();
    RenderingSet();   
}

// ���[�g�V�O�l�`���ݒ�
HRESULT SkyLUT::CreateRootSignature()
{    
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // participatingMedia�p�����[�_
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1); // SkyLUTBuffer�p�����[�_
    descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // ShadowFactor�e�N�X�`��

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

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 3;
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

//  �V�F�[�_�[�ݒ�
HRESULT SkyLUT::ShaderCompile()
{
    std::string vs = "SkyLUTVertex.hlsl";
    std::string ps = "SkyLUTPixel.hlsl";
    auto pair = Utility::GetHlslFilepath(vs, ps);

    auto result = D3DCompileFromFile
    (
        pair.first,
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
        pair.second,
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
    //desc.InputLayout.pInputElementDescs = inputLayout;
    desc.InputLayout.NumElements = /*_countof(inputLayout)*/0;
    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // model
    desc.SampleDesc.Count = 1; //1�T���v��/�s�N�Z��
    desc.SampleDesc.Quality = 0;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE/*D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT*/;
    desc.DepthStencilState.DepthEnable = false;
    //desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // �[�x�o�b�t�@�[�ɐ[�x�l��`������
    //desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // �\�[�X�f�[�^���R�s�[��f�[�^��菬�����ꍇ��������
    //desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget�ݒ�
void SkyLUT::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingCBVSRV();
}

// RenderingTarget RTV,SRV�p�q�[�v�̐���
HRESULT SkyLUT::CreateRenderingHeap()
{
    // RTV�p
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // RTV�p�f�B�X�N���v�^�q�[�v
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.ReleaseAndGetAddressOf()));
    if (result != S_OK) 
        return result;

    // SRV�p
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    srvHeapDesc.NodeMask = 0;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(cbvsrvHeap.ReleaseAndGetAddressOf()));
    
    return result;
}

// RenderingTarget�p���\�[�X�̐���
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

// RenderingTarget�pRTV�̍쐬
void SkyLUT::CreateRenderingRTV()
{
    auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;    

    _dev->CreateRenderTargetView//���\�[�X�f�[�^(_backBuffers)�ɃA�N�Z�X���邽�߂̃����_�[�^�[�Q�b�g�r���[��handle�A�h���X�ɍ쐬
    (
        renderingResource.Get(),//�����_�[�^�[�Q�b�g��\�� ID3D12Resource �I�u�W�F�N�g�ւ̃|�C���^�[
        &rtvDesc,//�����_�[ �^�[�Q�b�g �r���[���L�q���� D3D12_RENDER_TARGET_VIEW_DESC �\���̂ւ̃|�C���^�[�B
        handle//�V�����쐬���ꂽ�����_�[�^�[�Q�b�g�r���[�����݂��鈶���\�� CPU �L�q�q�n���h��(�q�[�v��̃A�h���X)
    );
}

// RenderingTarget�pSRV�̐���
void SkyLUT::CreateRenderingCBVSRV()
{
    auto handle = cbvsrvHeap->GetCPUDescriptorHandleForHeapStart();
    //auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    //// ParticipatingMedia�pview
    //D3D12_CONSTANT_BUFFER_VIEW_DESC pMediaCbvDesc = {};
    //pMediaCbvDesc.BufferLocation = participatingMediaResource->GetGPUVirtualAddress();
    //pMediaCbvDesc.SizeInBytes = participatingMediaResource->GetDesc().Width;
    //_dev->CreateConstantBufferView
    //(
    //    &pMediaCbvDesc,
    //    handle
    //);

    //handle.ptr += inc;

    // ShadowFactor�p
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

// �֗^�}���̐ݒ�
void SkyLUT::InitParticipatingMedia()
{
    CreateParticipatingResource();
    CreateParticipatingMediaHeapAndView();
    MappingParticipatingMedia();
}

// �֗^�}���p���\�[�X�̐���
HRESULT SkyLUT::CreateParticipatingResource()
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

    // SkyLUTBuffer�p
    desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SkyLUTBuffer) + 0xff) & ~0xff);
    result = _dev->CreateCommittedResource
    (
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(skyLUTBufferResource.ReleaseAndGetAddressOf())
    );

    return result;
}

// �֗^�}���p�q�[�v�E�r���[�̐���
HRESULT SkyLUT::CreateParticipatingMediaHeapAndView()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV�p�f�B�X�N���v�^�q�[�v
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 3; // CBV of ParticipatingMedia, SkyLUTBuffer
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(skyLUTHeap.ReleaseAndGetAddressOf()));
    if (result != S_OK) return result;

    auto handle = skyLUTHeap->GetCPUDescriptorHandleForHeapStart();
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

    // SkyLUTBuffer�pview
    D3D12_CONSTANT_BUFFER_VIEW_DESC SkyLUTCbvDesc = {};
    SkyLUTCbvDesc.BufferLocation = skyLUTBufferResource->GetGPUVirtualAddress();
    SkyLUTCbvDesc.SizeInBytes = skyLUTBufferResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &SkyLUTCbvDesc,
        handle
    );

    handle.ptr += inc;

    // SkyLUTBuffer�pview
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        shadowFactorResource.Get(),
        &srvDesc,
        handle
    );
}

// �֗^�}���p�萔�̃}�b�s���O
void SkyLUT::MappingParticipatingMedia()
{
    // ParticipatingMedia
    auto result = participatingMediaResource->Map(0, nullptr, (void**)&m_Media);

    // SkyLUTBuffer
    result = skyLUTBufferResource->Map(0, nullptr, (void**)&m_SkyLUT);
}

// �O������̊֗^�}���ݒ�
void SkyLUT::SetParticipatingMedia(ParticipatingMedia media)
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

// �O�������SkyLUTBuffer�ݒ�
void SkyLUT::SetSkyLUTBuffer(SkyLUTBuffer buffer)
{
    m_SkyLUT->eyePos = buffer.eyePos;
    m_SkyLUT->eyePos.y = 600.0f;
    m_SkyLUT->sunDirection = buffer.sunDirection;
    m_SkyLUT->stepCnt = buffer.stepCnt;
    m_SkyLUT->sunIntensity = buffer.sunIntensity;

    //m_SkyLUT->width = width;
    //m_SkyLUT->height = height;
}

void SkyLUT::SetShadowFactorResource(ID3D12Resource* _shadowFactorRsource)
{
    shadowFactorResource = _shadowFactorRsource;
    auto handle = skyLUTHeap->GetCPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    handle.ptr += inc * 2;

    // SkyLUTBuffer�pview
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _dev->CreateShaderResourceView
    (
        shadowFactorResource.Get(),
        &srvDesc,
        handle
    );
}

void SkyLUT::SetSkyLUTResolution()
{
    m_SkyLUT->width = width;
    m_SkyLUT->height = height;
}

void SkyLUT::ChangeSkyLUTResolution(int _width, int _height)
{
    m_SkyLUT->width = _width;
    m_SkyLUT->height = _height;
    
    width = _width;
    height = _height;

    RecreatreSource();
}

void SkyLUT::RecreatreSource()
{
    renderingResource.Get()->Release();
    renderingResource = nullptr;
    rtvHeap.Get()->Release();
    rtvHeap = nullptr;
    cbvsrvHeap.Get()->Release();
    cbvsrvHeap = nullptr;

    RenderingSet();
}

// ���s
void SkyLUT::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
    auto barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    _cmdList->RSSetViewports(1, _viewPort);
    _cmdList->RSSetScissorRects(1, _rect);

    //auto dsvhFBX = resourceManager[0]->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    //dsvhFBX.ptr += num * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _cmdList->OMSetRenderTargets(1, &heapHandle, false, nullptr);
    float clearColor[4] = { 1.0f,1.0f,1.0f,1.0f };
    _cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);
    //_cmdList->ClearDepthStencilView(dsvhFBX, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, skyLUTHeap.GetAddressOf());

    auto handle = skyLUTHeap->GetGPUDescriptorHandleForHeapStart();
    _cmdList->SetGraphicsRootDescriptorTable(0, handle);
    handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(1, handle);
    handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(2, handle);

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _cmdList->DrawInstanced(4, 1, 0, 0);

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    if (barrierSW)
    {
        // ShadowFactor�o�͂̃R�s�[�p���\�[�X��Ԃ�Compute Shader�œǂݏ����o�����Ԃɖ߂�
        auto barrierDescOfCopyDestTexture = CD3DX12_RESOURCE_BARRIER::Transition
        (
            shadowFactorResource.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );
        _cmdList->ResourceBarrier(1, &barrierDescOfCopyDestTexture);

        barrierSW = false;
    }
}

void SkyLUT::SetBarrierSWTrue()
{
    barrierSW = true;
}
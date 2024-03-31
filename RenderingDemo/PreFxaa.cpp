#include <stdafx.h>
#include <PreFxaa.h>


PreFxaa::PreFxaa(ID3D12Device* _dev, Integration* _integration, int _width, int _height) :
    _dev(_dev), pipelineState(nullptr), srvHeap(nullptr), renderingResource(nullptr)
{
    m_integration = _integration;
    width = _width;
    height = _height;
    Init();
    //scneMatrix = new SceneMatrix;
}

PreFxaa::~PreFxaa()
{
}

// ������
void PreFxaa::Init()
{
    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();
    RenderingSet();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT PreFxaa::CreateRootSignature()
{
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
    descTableRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
    descTableRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);

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

    // �v���O�����I�����Ƀf�X�g���N�^�Ăяo�����s�����A���̍ۂ�ComPtr������Q�Ɗm�F���s���Ă����B����ȑO�Ɉȉ��̂悤��Release���邱�Ƃ�client.h��InternalRelease()�ł�Release()���s�Ɏ��s���ăN���b�V������B
    // �܂�ARelease()��ComPtr�ɂ��C������ׂ�
    //rootSigBlob->Release();

    return result;
}

//  �V�F�[�_�[�ݒ�
HRESULT PreFxaa::ShaderCompile()
{
    std::string vs = "PeraVertex.hlsl";
    std::string ps = "PeraPixel.hlsl";
    auto pair = Utility::GetHlslFilepath(vs, ps);

    auto result = D3DCompileFromFile
    (
        pair.first,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs",
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
        "ps",
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
void PreFxaa::SetInputLayout()
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
HRESULT PreFxaa::CreateGraphicPipeline()
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
    desc.DepthStencilState.DepthEnable = false;
    //desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // �[�x�o�b�t�@�[�ɐ[�x�l��`������
    //desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // �\�[�X�f�[�^���R�s�[��f�[�^��菬�����ꍇ��������
    //desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    auto result = _dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget�ݒ�
void PreFxaa::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
}

// RenderingTarget RTV,SRV�p�q�[�v�̐���
HRESULT PreFxaa::CreateRenderingHeap()
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

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    return result;
}

// RenderingTarget�p���\�[�X�̐���
HRESULT PreFxaa::CreateRenderingResource()
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
void PreFxaa::CreateRenderingRTV()
{
    auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    _dev->CreateRenderTargetView
    (
        renderingResource.Get(),
        &rtvDesc,
        handle
    );
}

// RenderingTarget�pSRV�̐���
void PreFxaa::CreateRenderingSRV()
{
    auto handle = srvHeap->GetCPUDescriptorHandleForHeapStart();

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

// �O��srv
//void PreFxaa::SetSRHeapAndSRV(ComPtr<ID3D12DescriptorHeap> srvHeap)
//{
//    extSrvHeap = srvHeap;
//}

// ���s
void PreFxaa::Execution(ID3D12GraphicsCommandList* _cmdList, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
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
    _cmdList->SetDescriptorHeaps(1, m_integration->GetSRVHeap().GetAddressOf());

    auto handle = m_integration->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart();
    auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(0, handle);
    handle.ptr += inc * 2;
    _cmdList->SetGraphicsRootDescriptorTable(1, handle);
    handle.ptr += inc;
    _cmdList->SetGraphicsRootDescriptorTable(2, handle);
    handle.ptr += inc;
    _cmdList->SetGraphicsRootDescriptorTable(3, handle);
    handle.ptr += inc;
    _cmdList->SetGraphicsRootDescriptorTable(4, handle);

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _cmdList->DrawInstanced(3, 1, 0, 0);

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);
}
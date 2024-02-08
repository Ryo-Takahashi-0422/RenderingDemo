#include <stdafx.h>
#include <Shadow.h>

Shadow::Shadow(ID3D12Device* dev)
{
    _dev = dev;
}

void Shadow::Init()
{
    CreateRootSignature();
    ShaderCompile();
    SetInputLayout();
    CreateGraphicPipeline();

    RenderingSet();
    InitWVPMatrixReosources();
}

// ���[�g�V�O�l�`���ݒ�
HRESULT Shadow::CreateRootSignature()
{
    //�T���v���[�쐬
    stSamplerDesc[0].Init(0);
    stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    //�f�B�X�N���v�^�e�[�u���̃X���b�g�ݒ�
    descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // WVP

    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
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
HRESULT Shadow::ShaderCompile()
{
    std::string vs = "ShadowVertex.hlsl";
    std::string ps = "ShadowPixel.hlsl";
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
void Shadow::SetInputLayout()
{
    // ���W
    inputLayout =
    {
        {
        "POSITION",
        0, // �����Z�}���e�B�N�X�ɑ΂���C���f�b�N�X
        DXGI_FORMAT_R32G32B32_FLOAT,
        0, // �X���b�g�C���f�b�N�X
        D3D12_APPEND_ALIGNED_ELEMENT,
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        0 // ��x�ɕ`�悷��C���X�^���X��

        },

        //�@���x�N�g��
        {
            "NORMAL_Vertex",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },

        //uv
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0, // �X���b�g�C���f�b�N�X
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },

        //�{�[���ԍ�1�Z�b�g��
        {
            "BONE_NO_ZeroToTwo",
            0,
            DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[0]-bone[2]
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },

        //�{�[���ԍ�2�Z�b�g��
        {
            "BONE_NO_ThreeToFive",
            0,
            DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[3]-bone[5]
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },

        //�{�[���E�F�C�g1�Z�b�g��
        {
            "WEIGHT_ZeroToTwo",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT, // float weight[0] - [2]
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },

        //�{�[���E�F�C�g2�Z�b�g��
        {
            "WEIGHT_ThreeToFive",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT, // float weight[3] - [5]
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };
}

// �p�C�v���C���̐���
HRESULT Shadow::CreateGraphicPipeline()
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
    desc.InputLayout.pInputElementDescs = &inputLayout[0];
    desc.InputLayout.NumElements = inputLayout.size();
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
void Shadow::RenderingSet()
{
    CreateRenderingHeap();
    CreateRenderingResource();
    CreateRenderingRTV();
    CreateRenderingSRV();
    CreateRenderingDSV();
}

// RenderingTarget RTV,SRV,DSV�p�q�[�v�̐���
HRESULT Shadow::CreateRenderingHeap()
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
    srvHeapDesc.NumDescriptors = 1; // matrix, shadowfactor tex
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = _dev->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.ReleaseAndGetAddressOf()));

    // DSV�p
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1; // �ʏ�̐[�x�}�b�v
    result = _dev->CreateDescriptorHeap
    (
        &dsvHeapDesc,
        IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf())
    );

    return result;
}

// RenderingTarget�p���\�[�X�̐���
HRESULT Shadow::CreateRenderingResource()
{
    // �����_�����O�p
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

    // �[�x�}�b�v�p
    auto depthHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC depthResDesc = {};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResDesc.Width = width;
    depthResDesc.Height = height;
    depthResDesc.DepthOrArraySize = 1;
    depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS; // �[�x�l�������ݗp
    depthResDesc.SampleDesc.Count = 1; // 1pixce/1�̃T���v��
    depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE dValue = {};
    dValue.DepthStencil.Depth = 1.0f;
    dValue.Format = DXGI_FORMAT_D32_FLOAT;

    result = _dev->CreateCommittedResource
    (
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthResDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &dValue,
        IID_PPV_ARGS(depthBuff.ReleaseAndGetAddressOf())
    );

    return result;
}

// RenderingTarget�pRTV�̍쐬
void Shadow::CreateRenderingRTV()
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
void Shadow::CreateRenderingSRV()
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

// RenderingTarget�pDSV�̍쐬
void Shadow::CreateRenderingDSV()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    auto dsvHandle = dsvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
    // sponza�[�x�}�b�v�p
    _dev->CreateDepthStencilView
    (
        depthBuff.Get(),
        &dsvDesc,
        dsvHandle
    );
}

// WVP�ݒ�
void Shadow::InitWVPMatrixReosources()
{
    CreateWVPMatrixHeap();
    CreateWVPMatrixResources();
    CreateWVPMatrixView();
    MappingWVPMatrix();
}


HRESULT Shadow::CreateWVPMatrixHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; // SRV�p�f�B�X�N���v�^�q�[�v
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1; // matrix
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(matrixHeap.ReleaseAndGetAddressOf()));
    return result;
}

HRESULT Shadow::CreateWVPMatrixResources()
{
    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(WVPMatrix) + 0xff) & ~0xff);

    auto result = _dev->CreateCommittedResource
    (
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�q�[�v�ł̃��\�[�X������Ԃ͂��̃^�C�v���������[��
        nullptr,
        IID_PPV_ARGS(matrixResource.ReleaseAndGetAddressOf())
    );

    return result;
}

void Shadow::CreateWVPMatrixView()
{
    auto handle = matrixHeap->GetCPUDescriptorHandleForHeapStart();

    // frustum�pview
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = matrixResource->GetGPUVirtualAddress();
    desc.SizeInBytes = matrixResource->GetDesc().Width;
    _dev->CreateConstantBufferView
    (
        &desc,
        handle
    );
}

HRESULT Shadow::MappingWVPMatrix()
{
    auto result = matrixResource->Map(0, nullptr, (void**)&mappedMatrix);
    return result;
}

void Shadow::SetVertexAndIndexInfo(std::vector<D3D12_VERTEX_BUFFER_VIEW*> _vbViews, std::vector<D3D12_INDEX_BUFFER_VIEW*> _ibViews, std::vector<std::vector<std::pair<std::string, VertexInfo>>::iterator> _itIndiceFirsts, std::vector<std::vector<std::pair<std::string, VertexInfo>>> _indiceContainer)
{
    vbViews = _vbViews;
    ibViews = _ibViews;
    itIndiceFirsts = _itIndiceFirsts;
    indiceContainer = _indiceContainer;
}

void Shadow::SetVPMatrix(XMMATRIX _sunView, XMMATRIX _sunProj)
{
    mappedMatrix->world = XMMatrixIdentity();
    mappedMatrix->view = _sunView;
    mappedMatrix->proj = _sunProj;
}

void Shadow::SetBoneMatrix(FBXSceneMatrix* _fbxSceneMatrix)
{
    std::copy(std::begin(_fbxSceneMatrix->bones), std::end(_fbxSceneMatrix->bones), mappedMatrix->bones);
}

void Shadow::SetMoveMatrix(double speed, XMMATRIX charaDirection)
{
    // ���s�ړ������ɃL�����N�^�[�̌��������]��������Z���ĕ����ς��B����ɂ���]�ړ������͕s�v�Ȃ̂ŁA1��0�ɂ���BY����]�̂ݑΉ����Ă���B
    auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, -speed), charaDirection);
    moveMatrix.r[0].m128_f32[0] = 1;
    moveMatrix.r[0].m128_f32[2] = 0;
    moveMatrix.r[2].m128_f32[0] = 0;
    moveMatrix.r[2].m128_f32[2] = 1;
    moveMatrix.r[3].m128_f32[2] *= -1;

    m_moveMatrix *= moveMatrix;
}

void Shadow::SetRotationMatrix(XMMATRIX rotationMatrix)
{
    m_rotationMatrix *= rotationMatrix;
}

// ���s
void Shadow::Execution(ID3D12CommandQueue* _cmdQueue, ID3D12CommandAllocator* _cmdAllocator, ID3D12GraphicsCommandList* _cmdList, UINT64 _fenceVal, const D3D12_VIEWPORT* _viewPort, const D3D12_RECT* _rect)
{
    // ������ŃL�����N�^�[�̈ړ��p�s����X�V����
    mappedMatrix->world = XMMatrixMultiply(mappedMatrix->world, m_rotationMatrix);
    mappedMatrix->world = XMMatrixMultiply(mappedMatrix->world, m_moveMatrix);
    

    auto barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);



    _cmdList->RSSetViewports(1, _viewPort);
    _cmdList->RSSetScissorRects(1, _rect);

    auto dsvh = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    auto heapHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    _cmdList->OMSetRenderTargets(1, &heapHandle, false, &dsvh);
    float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };
    _cmdList->ClearRenderTargetView(heapHandle, clearColor, 0, nullptr);
    _cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // �[�x�o�b�t�@�[���N���A

    _cmdList->SetGraphicsRootSignature(rootSignature.Get());
    _cmdList->SetPipelineState(pipelineState.Get());
    _cmdList->SetDescriptorHeaps(1, matrixHeap.GetAddressOf());

    auto handle = matrixHeap->GetGPUDescriptorHandleForHeapStart();
    //auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _cmdList->SetGraphicsRootDescriptorTable(0, handle); // matrix

    _cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    
    for (int i = 0; i < 2; ++i)
    {
        int ofst = 0;
        int indiceSize = 0;
        auto itIndiceFirst = itIndiceFirsts[i];
        int indiceContainerSize = indiceContainer[i].size();
        _cmdList->IASetVertexBuffers(0, 1, vbViews[i]);
        _cmdList->IASetIndexBuffer(ibViews[i]);

        for (int j = 0; j < indiceContainerSize; ++j)
        {
            indiceSize = itIndiceFirst->second.indices.size(); // ���T�C�Y�݂̂�array��p�ӂ��Ă݂�
            _cmdList->DrawIndexedInstanced(indiceSize, 1, ofst, 0, 0);
            ofst += indiceSize;
            ++itIndiceFirst;
        }
    }

    barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
    (
        renderingResource.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc);

    // �f�u�X�}�b�v��ǂݍ��݉\��ԂɕύX����
    D3D12_RESOURCE_BARRIER barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
    (
        depthBuff.Get(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    _cmdList->ResourceBarrier(1, &barrierDesc4DepthMap);

    // �f�u�X�}�b�v��ǂݍ��݉\��ԂɕύX����
    //barrierDesc4DepthMap = CD3DX12_RESOURCE_BARRIER::Transition
    //(
    //    depthBuff.Get(),
    //    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
    //    D3D12_RESOURCE_STATE_DEPTH_WRITE
    //);
    //_cmdList->ResourceBarrier(1, &barrierDesc4DepthMap);
}
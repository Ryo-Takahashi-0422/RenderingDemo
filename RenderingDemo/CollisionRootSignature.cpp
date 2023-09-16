#include <stdafx.h>
#include <CollisionRootSignature.h>

HRESULT CollisionRootSignature::SetRootsignatureParam(ComPtr<ID3D12Device> _dev) {
	//�����\�[�X������
	// ����������1�F���[�g�V�O�l�`���ݒ�

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

	
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = rootParam;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = stSamplerDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	result = D3D12SerializeRootSignature //�V���A����
	(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		_rootSigBlob.ReleaseAndGetAddressOf(),
		_errorBlob.GetAddressOf()
	);

	result = _dev->CreateRootSignature
	(
		0,
		_rootSigBlob->GetBufferPointer(),
		_rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(_rootSignature.ReleaseAndGetAddressOf())
	);

	_rootSigBlob->Release();

	return S_OK;
}

CollisionRootSignature::~CollisionRootSignature()
{
}

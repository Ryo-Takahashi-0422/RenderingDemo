#pragma once

class SetRootSignature : public SetRootSignatureBase
{
private:
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[3] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[11] = {};
	D3D12_ROOT_PARAMETER rootParam[11] = {};

public:
	HRESULT SetRootsignatureParam(ComPtr<ID3D12Device> _dev);
	~SetRootSignature();
};
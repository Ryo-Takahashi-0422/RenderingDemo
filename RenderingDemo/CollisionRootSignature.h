#pragma once

class CollisionRootSignature : public SetRootSignatureBase
{
private:
	CD3DX12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
	CD3DX12_DESCRIPTOR_RANGE descTableRange[1] = {};
	D3D12_ROOT_PARAMETER rootParam[1] = {};

public:
	HRESULT SetRootsignatureParam(ComPtr<ID3D12Device> _dev);
	~CollisionRootSignature();
};
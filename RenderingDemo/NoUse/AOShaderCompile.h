#pragma once

class AOShaderCompile
{
private:
	HRESULT result;
	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> blobs;

public:
	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> SetShaderCompile
	(SetRootSignatureBase* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob);
};
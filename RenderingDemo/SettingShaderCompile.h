#pragma once
#include <PeraSetRootSignature.h>
#include <CollisionRootSignature.h>
class SettingShaderCompile
{
private:
	HRESULT result;
	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> blobs;

public:
	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> SetShaderCompile
	(SetRootSignature* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob, LPCWSTR vsPath, LPCSTR vsEntryPoint, LPCWSTR psPath, LPCSTR psEntryPoint);

	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> PeraSetShaderCompile
	(PeraSetRootSignature* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob, LPCWSTR vsPath, LPCSTR vsEntryPoint, LPCWSTR psPath, LPCSTR psEntryPoint);

	std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> CollisionSetShaderCompile
	(CollisionRootSignature* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob, LPCWSTR vsPath, LPCSTR vsEntryPoint, LPCWSTR psPath, LPCSTR psEntryPoint);

	~SettingShaderCompile();
};
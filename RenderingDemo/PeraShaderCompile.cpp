#include <stdafx.h>
#include <PeraShaderCompile.h>

std::pair<ComPtr<ID3D10Blob>, ComPtr<ID3D10Blob>> PeraShaderCompile::SetPeraShaderCompile
(SetRootSignatureBase* setPeraRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob)
{
	result = D3DCompileFromFile
	(
		//L"PeraVertex.hlsl",
		L"C:\\Users\\takataka\\source\\repos\\DirectX12_Play\\DirectX12_Play\\PeraVertex.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs",
		"vs_5_0",
		/*D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION*/0,
		0,
		_vsBlob.ReleaseAndGetAddressOf()
		, setPeraRootSignature->GetErrorBlob().GetAddressOf()
	);

	result = D3DCompileFromFile
	(
		//L"PeraPixel.hlsl",
		L"C:\\Users\\takataka\\source\\repos\\DirectX12_Play\\DirectX12_Play\\PeraPixel.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps",
		"ps_5_0",
		/*D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION*/0,
		0,
		_psBlob.ReleaseAndGetAddressOf()
		, setPeraRootSignature->GetErrorBlob().GetAddressOf()
	);

	//エラーチェック
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見つかりません");
			//return 0;
			blobs.first = nullptr;
			blobs.second = nullptr;
			return blobs;
		}
		else
		{
			std::string errstr;
			errstr.resize(setPeraRootSignature->GetErrorBlob()->GetBufferSize());

			std::copy_n((char*)setPeraRootSignature->GetErrorBlob()->GetBufferPointer(),
				setPeraRootSignature->GetErrorBlob()->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
	}

	blobs.first = _vsBlob;
	blobs.second = _psBlob;
	return blobs;
}
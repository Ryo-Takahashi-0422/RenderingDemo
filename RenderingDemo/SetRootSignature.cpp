#include <stdafx.h>
#include <SetRootSignature.h>

HRESULT SetRootSignature::SetRootsignatureParam(ComPtr<ID3D12Device> _dev) {
	//●リソース初期化
	// 初期化処理1：ルートシグネチャ設定

	//サンプラー作成
	stSamplerDesc[0].Init(0);
	stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	stSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	stSamplerDesc[1].Init(1, D3D12_FILTER_ANISOTROPIC,
	D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	stSamplerDesc[2].Init(2);
	stSamplerDesc[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	stSamplerDesc[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	stSamplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//stSamplerDesc[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//stSamplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // <=ならtrue(1.0) でなければfalse(0.0)
	//stSamplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // 比較結果をバイリニア補間
	//stSamplerDesc[2].MaxAnisotropy = 1; // 深度傾斜を有効にする
	//stSamplerDesc[2].ShaderRegister = 2;

	//ディスクリプタテーブルのスロット設定
	descTableRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // martix
	descTableRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 32, 1); // material b1-b33
	//descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0); // 描画する際に参照するsレジスタ番号群
	descTableRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // color
	descTableRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // normal
	descTableRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2); // specular
	descTableRange[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3); // metal
	descTableRange[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4); // color
	descTableRange[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5); // normal
	descTableRange[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6); // specular
	descTableRange[9].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7); // metal
	descTableRange[10].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8); // transparency
	descTableRange[11].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);
	descTableRange[12].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);

	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1; // WVP用
	rootParam[0].DescriptorTable.pDescriptorRanges = descTableRange;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//rootParam[1].DescriptorTable.NumDescriptorRanges = 2; // マテリアル(CBV)とテクスチャ(SRV)の1セット5個で使う
	//rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1]; // ここからNumDescriptorRange分、つまり[1]と[2]が該当する。
	//rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1; // phong material info
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[2].DescriptorTable.pDescriptorRanges = &descTableRange[2]; 
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[3].DescriptorTable.pDescriptorRanges = &descTableRange[3];
	rootParam[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[4].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[4].DescriptorTable.pDescriptorRanges = &descTableRange[4];
	rootParam[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	rootParam[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[5].DescriptorTable.pDescriptorRanges = &descTableRange[5];
	rootParam[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[6].DescriptorTable.pDescriptorRanges = &descTableRange[6];
	rootParam[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[7].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[7].DescriptorTable.pDescriptorRanges = &descTableRange[7];
	rootParam[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[8].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[8].DescriptorTable.pDescriptorRanges = &descTableRange[8];
	rootParam[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[9].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[9].DescriptorTable.pDescriptorRanges = &descTableRange[9];
	rootParam[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[10].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[10].DescriptorTable.pDescriptorRanges = &descTableRange[10];
	rootParam[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[11].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[11].DescriptorTable.pDescriptorRanges = &descTableRange[11];
	rootParam[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParam[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[12].DescriptorTable.NumDescriptorRanges = 1; 
	rootParam[12].DescriptorTable.pDescriptorRanges = &descTableRange[12];
	rootParam[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootSignatureDesc.NumParameters = 13;
	rootSignatureDesc.pParameters = rootParam;
	rootSignatureDesc.NumStaticSamplers = 3;
	rootSignatureDesc.pStaticSamplers = stSamplerDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	result = D3D12SerializeRootSignature //シリアル化
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

	//_rootSigBlob->Release();

	return S_OK;
}

SetRootSignature::~SetRootSignature()
{
	// ComPtr解放のみ
}

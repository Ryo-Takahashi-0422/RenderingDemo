#pragma once
#include <SettingImgui.h>

class MappingExecuter
{
private:
	PMDMaterialInfo* pmdMaterialInfo = nullptr;
	BufferHeapCreator* bufferHeapCreator = nullptr;

	// マッピング先ポインタ群
	unsigned char* vertMap = nullptr;
	unsigned short* mappedIdx = nullptr;
	char* mapMaterial = nullptr;
	uint8_t* mapforImg = nullptr;
	float* mappedweight = nullptr;
	
	PostSetting* mappedPostSetting;

	HRESULT result;

public:
	MappingExecuter(PMDMaterialInfo* _pmdMaterialInfo, BufferHeapCreator* _bufferHeapCreator);

	//頂点バッファーの仮想アドレスをポインタにマップ(関連付け)して、仮想的に頂点データをコピーする。
	void MappingVertBuff();

	//インデクスバッファーの仮想アドレスをポインタにマップ(関連付け)して、仮想的にインデックスデータをコピーする。
	void MappingIndexOfVertexBuff();

	//マテリアル用バッファーへのマッピング
	void MappingMaterialBuff();

	// テクスチャアップロード用バッファーの仮想アドレスをポインタにマップ(関連付け)して、仮想的にインデックスデータをコピーする。
	void TransferTexUploadToBuff(std::vector<ComPtr<ID3D12Resource>> uploadBuff, std::vector<DirectX::Image*> img, unsigned int itCount);

	// ガウシアンぼかしバッファーへのウェイト値マッピング
	void MappingGaussianWeight(std::vector<float> weights);

	// imgui PostSettingへのマッピング
	void MappingPostSetting();

	PostSetting* GetMappedPostSetting() { return mappedPostSetting; };
};
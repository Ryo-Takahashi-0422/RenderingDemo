#include <stdafx.h>
#include <MappingExecuter.h>

MappingExecuter::MappingExecuter(PMDMaterialInfo* _pmdMaterialInfo, BufferHeapCreator* _bufferHeapCreator)
{
	pmdMaterialInfo = _pmdMaterialInfo;
	bufferHeapCreator = _bufferHeapCreator;
	mappedPostSetting = new PostSetting{};
}

void MappingExecuter::MappingVertBuff()
{
	//CPUは暗黙的なヒープの情報を得られないため、Map関数によりVRAM上のバッファーにアドレスを割り当てた状態で
	//頂点などの情報をVRAMへコピーしている(次の３つはCPUとGPUどちらもアクセス可能なUPLOADタイプなヒープ故マップ可能)、
	//という理解。Unmapはコメントアウトしても特に影響はないが...
	result = bufferHeapCreator->GetVertBuff()->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(pmdMaterialInfo->vertices), std::end(pmdMaterialInfo->vertices), vertMap);
	bufferHeapCreator->GetVertBuff()->Unmap(0, nullptr);
}

void MappingExecuter::MappingIndexOfVertexBuff()
{
	result = bufferHeapCreator->GetIdxBuff()->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(pmdMaterialInfo->indices), std::end(pmdMaterialInfo->indices), mappedIdx);
	bufferHeapCreator->GetIdxBuff()->Unmap(0, nullptr);
}

void MappingExecuter::MappingMaterialBuff()
{
	result = bufferHeapCreator->GetMaterialBuff()->Map(0, nullptr, (void**)&mapMaterial);
	for (auto m : pmdMaterialInfo->materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;
		mapMaterial += bufferHeapCreator->GetMaterialBuffSize();
	}
	bufferHeapCreator->GetMaterialBuff()->Unmap(0, nullptr);
}

void MappingExecuter::TransferTexUploadToBuff(std::vector<ComPtr<ID3D12Resource>> uploadBuff, std::vector<DirectX::Image*> img, unsigned int itCount)
{
	for (int count = 0; count < itCount; count++)
	{
		if (uploadBuff[count] == nullptr) continue;

		auto srcAddress = img[count]->pixels;
		auto rowPitch = Utility::AlignmentSize(img[count]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		result = uploadBuff[count]->Map(0, nullptr, (void**)&mapforImg);

		// img:元データの初期アドレス(srcAddress)を元ピッチ分オフセットしながら、補正したピッチ個分(rowPitch)のアドレスを
		// mapforImgにその数分(rowPitch)オフセットを繰り返しつつコピーしていく
		for (int i = 0; i < img[count]->height; ++i)
		{
			std::copy_n(srcAddress, rowPitch, mapforImg);
			srcAddress += img[count]->rowPitch;
			mapforImg += rowPitch;
		}

		uploadBuff[count]->Unmap(0, nullptr);
	}
}

void MappingExecuter::MappingGaussianWeight(std::vector<float> weights)
{
	bufferHeapCreator->GetGaussianBuff()->Map(0, nullptr, (void**)&mappedweight);
	std::copy(weights.begin(), weights.end(), mappedweight);
	bufferHeapCreator->GetGaussianBuff()->Unmap(0, nullptr);
}

void MappingExecuter::MappingPostSetting()
{
	
	auto result = bufferHeapCreator->GetImguiPostSettingBuff()->Map(0, nullptr, (void**)&mappedPostSetting);
}
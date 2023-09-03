#pragma once

class TextureTransporter
{
private:
	FBXInfoManager* fbxInfoManager = nullptr;

	// PMDテクスチャ用転送オブジェクト
	std::vector<D3D12_TEXTURE_COPY_LOCATION> source;
	std::vector<D3D12_TEXTURE_COPY_LOCATION> dest;
	std::vector<D3D12_RESOURCE_BARRIER> texBarriierDesc;

public:
	TextureTransporter(FBXInfoManager* _fbxInfoManager);

	// テクスチャをitCount数分まとめてGPUのUpload用バッファからGPUのRead用バッファへデータコピー
	// 元はpmdマテリアル数分まとめて処理していたメソッドを共通化した
	void TransportPMDMaterialTexture
	(
		ComPtr<ID3D12GraphicsCommandList> _cmdList,
		ComPtr<ID3D12CommandAllocator> _cmdAllocator,
		ComPtr<ID3D12CommandQueue> _cmdQueue,
		std::vector<DirectX::TexMetadata*> metaData,
		std::vector<DirectX::Image*> img,
		ComPtr<ID3D12Fence> _fence,
		UINT64& _fenceVal,
		std::vector<ComPtr<ID3D12Resource>> uploadBuff,
		std::vector<ComPtr<ID3D12Resource>> readBuff
	);
};
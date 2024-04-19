#pragma once
//using namespace Microsoft::WRL;
//using namespace DirectX;

class CreateD3DX12ResourceBuffer 
{
private:


public:

	// テクスチャ用　CPUからのアップロード用バッファ、GPUからの読み取り用バッファ、DirectX::Image生成
	// @param metaData ロードしたファイルのTexmetadataオブジェクト
	// @param img ロードしたファイルのImageオブジェクト
	// @return CPUからのアップロード用バッファ,GPUからの読み取り用バッファ,DirectX::TexMetadata,DirectX::Image
	static std::tuple<ComPtr<ID3D12Resource>, ComPtr<ID3D12Resource>> LoadTextureFromFile
	(
		ComPtr<ID3D12Device> _dev,
		TexMetadata* metaData,
		Image* img
	);

	static ComPtr<ID3D12Resource> LoadTextureFromFile4UploadFile
	(
		ComPtr<ID3D12Device> _dev,
		Image* img
	);

	static ComPtr<ID3D12Resource> CreateMappedSphSpaTexResource
	(
		ComPtr<ID3D12Device> _dev, 
		TexMetadata* metaData, 
		Image* img, std::string 
		texPath
	);

	static ComPtr<ID3D12Resource> CreateColorTexture(ComPtr<ID3D12Device> _dev, const int param);

	//トゥーンのためのグラデーションテクスチャ
	static ComPtr<ID3D12Resource> CreateGrayGradationTexture(ComPtr<ID3D12Device> _dev);
};
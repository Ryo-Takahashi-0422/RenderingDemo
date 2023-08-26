#pragma once

class BufferHeapCreator
{
private:
	PMDMaterialInfo* pmdMaterialInfo = nullptr;
	PrepareRenderingWindow* prepareRenderingWindow = nullptr;
	TextureLoader* textureLoader = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; // レンダーターゲットビュー用ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {}; // 深度ステンシルビュー用ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {}; // 行列用ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC mutipassRTVHeapDesc = {}; // マルチパスRTV用ディスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC mutipassSRVHeapDesc = {}; // マルチパスSRV用ディスクリプタヒープ

	ComPtr<ID3D12DescriptorHeap> rtvHeaps = nullptr; // リソースターゲットビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> cbvsrvHeap = nullptr; // 行列CBV, SRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> multipassRTVHeap = nullptr; // マルチパスレンダーターゲット用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> multipassSRVHeap = nullptr; // マルチパステクスチャ用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> spriteFontHeap = nullptr; // DirectXTK SpriteFont用ディスクリプタヒープ

	D3D12_HEAP_PROPERTIES vertexHeapProps = {}; // 頂点、頂点インデックス用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProps = {}; // 深度用ヒーププロパティ
	D3D12_HEAP_PROPERTIES lightMapHeapProps = {}; // ライトマップバッファー用ヒーププロパティ
	D3D12_HEAP_PROPERTIES wvpHeapProp = {}; // 行列用ヒーププロパティ
	D3D12_HEAP_PROPERTIES materialHeapProp = {};  // PMDMaterialInfo->MaterialForHlsl定義に合わせている
	D3D12_HEAP_PROPERTIES mutipassHeapProp = {}; // マルチパス用ヒーププロパティ
	D3D12_HEAP_PROPERTIES gaussianHeapProp = {}; // ガウスぼかし用ヒーププロパティ

	D3D12_RESOURCE_DESC vertresDesc; // 頂点用リソース詳細
	D3D12_RESOURCE_DESC indicesDesc; // 頂点インデックス用リソース詳細
	D3D12_RESOURCE_DESC depthResDesc = {}; // 深度リソース詳細
	D3D12_RESOURCE_DESC lightMapResDesc = {}; // ライトマップ用バッファー詳細
	D3D12_RESOURCE_DESC wvpResdesc = {}; // 行列用リソース詳細
	D3D12_RESOURCE_DESC materialBuffResDesc = {}; // PMDMaterialInfo->MaterialForHlsl定義に合わせている
	// ﾏﾙﾁﾊﾟｽﾊﾞｯﾌｧ用のオブジェクトはAppD3DX12が生成しているため保持しない
	D3D12_RESOURCE_DESC gaussianBuffResDesc = {}; // ガウスぼかし用

	ComPtr<ID3D12Resource> vertBuff = nullptr; // 頂点用バッファ
	ComPtr<ID3D12Resource> idxBuff = nullptr; // 頂点インデックス用バッファ
	ComPtr<ID3D12Resource> depthBuff = nullptr; // デプスバッファー
	ComPtr<ID3D12Resource> depthBuff2 = nullptr; // デプスバッファー
	ComPtr<ID3D12Resource> lightMapBuff = nullptr; // ライトマップバッファー
	ComPtr<ID3D12Resource> matrixBuff = nullptr; // 行列用定数バッファー
	ComPtr<ID3D12Resource> matrixBuff4Multipass = nullptr; // ﾏﾙﾁﾊﾟｽ用行列用定数バッファー
	ComPtr<ID3D12Resource> materialBuff = nullptr; // マテリアル用定数バッファー
	ComPtr<ID3D12Resource> multipassBuff = nullptr; // マルチパスレンダリング用書き込み先バッファー
	ComPtr<ID3D12Resource> multipassBuff2 = nullptr; // マルチパスレンダリング用書き込み先バッファーその2
	ComPtr<ID3D12Resource> multipassBuff3 = nullptr; // マルチパスレンダリング用書き込み先バッファーその3
	ComPtr<ID3D12Resource> whiteTextureBuff = nullptr; // 白テクスチャ用バッファー
	ComPtr<ID3D12Resource> blackTextureBuff = nullptr; // 黒テクスチャ用バッファー
	ComPtr<ID3D12Resource> grayTextureBuff = nullptr; // グレーテクスチャ用バッファー
	ComPtr<ID3D12Resource> gaussianBuff = nullptr; // ガウスぼかし用バッファー
	std::array<ComPtr<ID3D12Resource>, 3> _bloomBuff; // ブルーム用バッファー+shrinkedModel
	ComPtr<ID3D12Resource> aoBuff = nullptr; // buffer for AO
	ComPtr<ID3D12Resource> imguiBuff = nullptr; // buffer for imgui rendering
	ComPtr<ID3D12Resource> imguiPostSettingBuff = nullptr; // buffer for imgui PostSetting

	std::vector<ComPtr<ID3D12Resource>> normalMapUploadBuff; // ノーマルマップ用アップロードバッファー
	std::vector<ComPtr<ID3D12Resource>> normalMapReadBuff; // ノーマルマップ用リードバッファー
	// PMDモデル用のメンバー
	std::vector<ComPtr<ID3D12Resource>> pmdTexUploadBuff; // PMDモデルのテクスチャCPUアップロード用バッファー
	std::vector<ComPtr<ID3D12Resource>> pmdTexReadBuff; // PMDモデルのテクスチャGPU読み取り用バッファー
	std::vector<ComPtr<ID3D12Resource>> sphMappedBuff; // sph用バッファー
	std::vector<ComPtr<ID3D12Resource>> spaMappedBuff; // spa用バッファー
	std::vector<ComPtr<ID3D12Resource>> toonUploadBuff; // トゥーン用アップロードバッファー
	std::vector<ComPtr<ID3D12Resource>> toonReadBuff; // トゥーン用リードバッファー

	D3D12_CLEAR_VALUE depthClearValue = {}; // 深度クリアバリュー

	std::string textureFolader = "texture";
	std::vector<DirectX::TexMetadata*> normalMapMetaData;
	std::vector<DirectX::Image*> normalMapImg;
	struct _stat s = {};
	ScratchImage scratchImg = {};
	ScratchImage normalMapScratchImg = {};
	ScratchImage toonScratchImg = {};
	std::string filePath; // テクスチャ保存フォルダのパス
	std::string toonFilePath; // トゥーンテクスチャ保存フォルダのパス
	HRESULT result;

	unsigned long materialBuffSize;

	


public:
	BufferHeapCreator(PMDMaterialInfo* _pmdMaterialInfo, PrepareRenderingWindow* _prepareRenderingWindow, TextureLoader* _textureLoader);
	// RTV用ディスクリプタヒープ設定
	void SetRTVHeapDesc();
	// DSV用ディスクリプタヒープ設定
	void SetDSVHeapDesc();
	// CBV,SRV用ディスクリプタヒープ設定(行列CBV、ﾏﾃﾘｱﾙCBV、ﾃｸｽﾁｬSRVをセット)
	void SetCBVSRVHeapDesc();
	// マルチパスRTV用ディスクリプタヒープ設定
	void SetMutipassRTVHeapDesc();
	// マルチパスSRV用ディスクリプタヒープ設定
	void SetMutipassSRVHeapDesc();
	// 頂点・インデックス用ヒーププロパティ設定
	void SetVertexHeapProp();
	// 深度用ヒーププロパティ設定
	void SetDepthHeapProp();
	// 深度用リソース詳細設定
	void SetDepthResourceDesc();
	// クリアバリュー設定
	void SetClearValue();

	// RTVディスクリプタヒープの作成
	HRESULT CreateRTVHeap(ComPtr<ID3D12Device> _dev);

	// DSVディスクリプタヒープの作成
	HRESULT CreateDSVHeap(ComPtr<ID3D12Device> _dev);

	// CBV,SRV兼用ディスクリプタヒープの作成
	HRESULT CreateCBVSRVHeap(ComPtr<ID3D12Device> _dev);

	// マルチパスRTV用ディスクリプタヒープの作成
	HRESULT CreateMultipassRTVHeap(ComPtr<ID3D12Device> _dev);

	// マルチパスSRV用ディスクリプタヒープの作成
	HRESULT CreateMultipassSRVHeap(ComPtr<ID3D12Device> _dev);

	// SpriteFont用ディスクリプタヒープの作成
	HRESULT CreateSpriteFontHeap(ComPtr<ID3D12Device> _dev);

	// 頂点バッファーの作成
	HRESULT CreateBufferOfVertex(ComPtr<ID3D12Device> _dev);

	// インデックスバッファーの作成
	HRESULT CreateBufferOfIndex(ComPtr<ID3D12Device> _dev);

	// デプスバッファーの作成
	HRESULT CreateBufferOfDepthAndLightMap(ComPtr<ID3D12Device> _dev);

	// 行列用定数バッファーの作成 WVP:World,View,Projection
	HRESULT CreateConstBufferOfWVPMatrix(ComPtr<ID3D12Device> _dev);

	// マテリアル用定数バッファーの作成。PMDMaterialInfo->MaterialForHlsl定義用。
	HRESULT CreateConstBufferOfMaterial(ComPtr<ID3D12Device> _dev);

	// マルチパスレンダリング用のバッファ作成
	HRESULT CreateRenderBufferForMultipass(ComPtr<ID3D12Device> _dev, D3D12_RESOURCE_DESC& mutipassResDesc);

	// ガウスぼかし用定数バッファーの作成。
	HRESULT CreateConstBufferOfGaussian(ComPtr<ID3D12Device> _dev, std::vector<float> weights);

	// 任意ファイル形式のノーマルマップ用CPU_Upload用、GPU_Read用バッファの作成
	void CreateUploadAndReadBuff4Normalmap(ComPtr<ID3D12Device> _dev,
		std::string strModelPath,
		std::string fileType,
		unsigned int texNum);

	// PMDモデルのテクスチャ用CPU_Upload用、GPU_Read用バッファの作成
	void CreateUploadAndReadBuff4PmdTexture(ComPtr<ID3D12Device> _dev,
		std::string strModelPath,
		std::vector<DirectX::TexMetadata*>& metaData,
		std::vector<DirectX::Image*>& img);

	// PMDモデルのToonテクスチャ用CPU_Upload用、GPU_Read用バッファの作成
	void CreateToonUploadAndReadBuff(ComPtr<ID3D12Device> _dev,
		std::string strModelPath,
		std::vector<DirectX::TexMetadata*>& toonMetaData,
		std::vector<DirectX::Image*>& toonImg);

	// 白黒グレー各テクスチャバッファ作成
	void CreateTextureBuffers(ComPtr<ID3D12Device> _dev);

	// imgui用にﾃﾞｨｽｸﾘﾌﾟﾀﾋｰﾌﾟ、ﾘｿｰｽを作成する
	HRESULT CreateBuff4Imgui(ComPtr<ID3D12Device> _dev, size_t sizeofPostSetting);
	
	// 外部公開用
	unsigned long GetMaterialBuffSize() { return materialBuffSize; };
	ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return rtvHeaps; };
	ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return dsvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetCBVSRVHeap() { return cbvsrvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetMultipassRTVHeap() { return multipassRTVHeap; };
	ComPtr<ID3D12DescriptorHeap> GetMultipassSRVHeap() { return multipassSRVHeap; };
	ComPtr<ID3D12DescriptorHeap> GetSpriteFontHeap() { return spriteFontHeap; };

	ComPtr<ID3D12Resource> GetVertBuff() { return vertBuff; };
	ComPtr<ID3D12Resource> GetIdxBuff() { return idxBuff; };
	ComPtr<ID3D12Resource> GetDepthBuff() { return depthBuff; };
	ComPtr<ID3D12Resource> GetDepthBuff2() { return depthBuff2; };
	ComPtr<ID3D12Resource> GetLightMapBuff() { return lightMapBuff; };
	ComPtr<ID3D12Resource> GetMatrixBuff() { return matrixBuff; };
	ComPtr<ID3D12Resource> GetMatrixBuff4Multipass() { return matrixBuff4Multipass; };
	ComPtr<ID3D12Resource> GetMaterialBuff() { return materialBuff; };
	ComPtr<ID3D12Resource> GetMultipassBuff() { return multipassBuff; };
	ComPtr<ID3D12Resource> GetMultipassBuff2() { return multipassBuff2; };
	ComPtr<ID3D12Resource> GetMultipassBuff3() { return multipassBuff3; };
	ComPtr<ID3D12Resource> GetWhiteTextureBuff() { return whiteTextureBuff; };
	ComPtr<ID3D12Resource> GetBlackTextureBuff() { return blackTextureBuff; };
	ComPtr<ID3D12Resource> GetGrayTextureBuff() { return grayTextureBuff; };
	ComPtr<ID3D12Resource> GetGaussianBuff() { return gaussianBuff; };
	ComPtr<ID3D12Resource> GetAOBuff() { return aoBuff; };
	ComPtr<ID3D12Resource> GetImguiBuff() { return imguiBuff; };
	ComPtr<ID3D12Resource> GetImguiPostSettingBuff() { return imguiPostSettingBuff; };

	std::vector<ComPtr<ID3D12Resource>> GetNormalMapUploadBuff() { return normalMapUploadBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetNormalMapReadBuff() { return normalMapReadBuff; };
	// 以下はPMDファイル用
	std::vector<ComPtr<ID3D12Resource>> GetPMDTexUploadBuff() { return pmdTexUploadBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetPMDTexReadBuff() { return pmdTexReadBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetsphMappedBuff() { return sphMappedBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetspaMappedBuff() { return spaMappedBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetToonUploadBuff() { return toonUploadBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetToonReadBuff() { return toonReadBuff; };
	std::array<ComPtr<ID3D12Resource>, 3> GetBloomBuff() { return _bloomBuff; };
	
	
	// texture情報群
	std::vector<DirectX::TexMetadata*> GetNormalMapMetadata() { return normalMapMetaData; };
	std::vector<DirectX::Image*> GetNormalMapImg() { return normalMapImg; };
};
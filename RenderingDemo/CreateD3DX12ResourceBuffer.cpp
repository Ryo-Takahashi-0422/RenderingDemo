#include <stdafx.h>
#include <CreateD3DX12ResourceBuffer.h>

std::tuple<ComPtr<ID3D12Resource>, ComPtr<ID3D12Resource>> 
CreateD3DX12ResourceBuffer::LoadTextureFromFile(ComPtr<ID3D12Device> _dev, TexMetadata* metaData, Image* img)
{	
	//メソッド内でimg等動的メモリ確保のポインタを返すことは不可能と理解した
	ComPtr<ID3D12Resource> texUploadBuff = nullptr;//テクスチャCPUアップロード用バッファー

	//テクスチャバッファー用のCPU特化型(ｱｯﾌﾟﾛｰﾄﾞﾀｲﾌﾟ)ヒーププロパティ設定
	D3D12_HEAP_PROPERTIES texUploadHeapProp;
	texUploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	texUploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texUploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texUploadHeapProp.CreationNodeMask = 0; // 単一アダプターのため
	texUploadHeapProp.VisibleNodeMask = 0; // 単一アダプターのため

	//アップロード用
	D3D12_RESOURCE_DESC texUploadResourceDesc = {};
	texUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	texUploadResourceDesc.Alignment = 0;
	texUploadResourceDesc.Width = Utility::AlignmentSize(img->rowPitch/*slicePitch*/, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;// *5;
	texUploadResourceDesc.Height = 1;
	texUploadResourceDesc.DepthOrArraySize = 1;
	texUploadResourceDesc.MipLevels = 1;
	texUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	texUploadResourceDesc.SampleDesc.Count = 1;
	texUploadResourceDesc.SampleDesc.Quality = 0;
	texUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	texUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//CPUからのアップロード用テクスチャバッファーを作成
	HRESULT result = _dev->CreateCommittedResource
	(&texUploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texUploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadタイプのヒープにおける推奨設定。CPUのみが書き込みを行っているからとの公式情報。
		nullptr,
		IID_PPV_ARGS(texUploadBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return std::forward_as_tuple(nullptr, nullptr);
	}


	ComPtr<ID3D12Resource> texReadBuff = nullptr;//テクスチャGPU読み取り用バッファー

	//テクスチャバッファー用のGPU特化型ヒーププロパティ設定
	D3D12_HEAP_PROPERTIES texReadHeapProp = {};
	texReadHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	texReadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texReadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texReadHeapProp.CreationNodeMask = 0; // 単一アダプターのため
	texReadHeapProp.VisibleNodeMask = 0; // 単一アダプターのため

	//GPU読み取り用
	D3D12_RESOURCE_DESC texReadResourceDesc = {};
	texReadResourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);
	texReadResourceDesc.Alignment = 0;
	texReadResourceDesc.Width = metaData->width;
	texReadResourceDesc.Height = metaData->height;
	texReadResourceDesc.DepthOrArraySize = metaData->arraySize;
	texReadResourceDesc.MipLevels = /*metaData->mipLevels*/5;
	texReadResourceDesc.Format = metaData->format;
	texReadResourceDesc.SampleDesc.Count = 1;
	texReadResourceDesc.SampleDesc.Quality = 0;
	texReadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texReadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//UINT64 textureUploadBufferSize;
	//_dev->GetCopyableFootprints(&texReadResourceDesc, 0, texReadResourceDesc.MipLevels, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);
	//auto desc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

	//GPUからの読み取り用テクスチャバッファーを作成
	result = _dev->CreateCommittedResource
	(&texReadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texReadResourceDesc/*&desc*/,
		D3D12_RESOURCE_STATE_COPY_DEST, // バッファーがCPUからのリソースコピー先であることを示す
		nullptr,
		IID_PPV_ARGS(texReadBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return std::forward_as_tuple(texUploadBuff.Get(), nullptr);
	}

	return std::forward_as_tuple(texUploadBuff.Get(), texReadBuff.Get());
}



ComPtr<ID3D12Resource> CreateD3DX12ResourceBuffer::LoadTextureFromFile4UploadFile(ComPtr<ID3D12Device> _dev, Image* img)
{
	//メソッド内でimg等動的メモリ確保のポインタを返すことは不可能と理解した
	ComPtr<ID3D12Resource> texUploadBuff = nullptr;//テクスチャCPUアップロード用バッファー

	//テクスチャバッファー用のCPU特化型(ｱｯﾌﾟﾛｰﾄﾞﾀｲﾌﾟ)ヒーププロパティ設定
	D3D12_HEAP_PROPERTIES texUploadHeapProp;
	texUploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	texUploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texUploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texUploadHeapProp.CreationNodeMask = 0; // 単一アダプターのため
	texUploadHeapProp.VisibleNodeMask = 0; // 単一アダプターのため

	//アップロード用
	D3D12_RESOURCE_DESC texUploadResourceDesc = {};
	texUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	texUploadResourceDesc.Alignment = 0;
	texUploadResourceDesc.Width = Utility::AlignmentSize(img->rowPitch/*slicePitch*/, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;// *5;
	texUploadResourceDesc.Height = 1;
	texUploadResourceDesc.DepthOrArraySize = 1;
	texUploadResourceDesc.MipLevels = 1;
	texUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	texUploadResourceDesc.SampleDesc.Count = 1;
	texUploadResourceDesc.SampleDesc.Quality = 0;
	texUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	texUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//CPUからのアップロード用テクスチャバッファーを作成
	HRESULT result = _dev->CreateCommittedResource
	(&texUploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texUploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadタイプのヒープにおける推奨設定。CPUのみが書き込みを行っているからとの公式情報。
		nullptr,
		IID_PPV_ARGS(texUploadBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	return texUploadBuff.Get();
}



ComPtr<ID3D12Resource> CreateD3DX12ResourceBuffer::CreateMappedSphSpaTexResource(ComPtr<ID3D12Device> _dev, TexMetadata* metaData, Image* img, std::string texPath)
{
	// FlyWeight Pattern：同一テクスチャからのリソース生成を防ぎ、既存のリソースを返す
	std::map<std::string, ComPtr<ID3D12Resource>> _resourceTable;
	auto iterator = _resourceTable.find(texPath);
	if (iterator != _resourceTable.end()) {
		return iterator->second;
	};

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);
	resDesc.Alignment = 0;
	resDesc.Width = metaData->width;
	resDesc.Height = metaData->height;
	resDesc.DepthOrArraySize = metaData->arraySize;
	resDesc.MipLevels = metaData->mipLevels;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// B8G8R8X8でsrv生成するとエラー metaData->format;
	//resDesc.Format = metaData->format;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> MappedBuff = nullptr;
	auto result = _dev->CreateCommittedResource
	(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(MappedBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	result = MappedBuff->WriteToSubresource
	(
		0,
		nullptr,
		img->pixels,
		img->rowPitch,
		img->slicePitch
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	_resourceTable[texPath] = MappedBuff;
	return MappedBuff;
}

ComPtr<ID3D12Resource> CreateD3DX12ResourceBuffer::CreateColorTexture(ComPtr<ID3D12Device> _dev, const int param)
{
	ComPtr<ID3D12Resource> whiteBuff = nullptr;
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto whiteResDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 4);
	HRESULT result = _dev->CreateCommittedResource
	(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&whiteResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(whiteBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return nullptr;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), param);

	result = whiteBuff->WriteToSubresource
	(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size() // ソースデータの1つの深度スライスから次のデータまでの距離、つまりサイズ
	);

	return whiteBuff;
}

ComPtr<ID3D12Resource> CreateD3DX12ResourceBuffer::CreateGrayGradationTexture(ComPtr<ID3D12Device> _dev)
{
	ComPtr<ID3D12Resource> grayBuff = nullptr;
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 4, 256);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(grayBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		return nullptr;
	}

	//上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0x30;
	int i = 0;
	for (; it != data.end(); it += 4) {
		auto col = (0000 << 24) | RGB(c, c, c);
		std::fill(it, it + 3, col);
		i++;
		if (i == 85)
		{
			c = 0xa0;
		}

		if (i == 170)
		{
			c = 0xff;
		}
	}

	result = grayBuff->WriteToSubresource
	(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * data.size()
	);

	return grayBuff;
}
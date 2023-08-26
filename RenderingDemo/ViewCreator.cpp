#include <stdafx.h>
#include <ViewCreator.h>

ViewCreator::ViewCreator(PMDMaterialInfo* _pmdMaterialInfo, BufferHeapCreator* _bufferHeapCreator)
	: pmdMaterialInfo(_pmdMaterialInfo), bufferHeapCreator(_bufferHeapCreator)
{}

void ViewCreator::CreateCBV4Matrix(ComPtr<ID3D12Device> _dev)
{
	SetCBVDesc4Matrix();

	_dev->CreateConstantBufferView
	(
		&cbvDesc,
		bufferHeapCreator->GetCBVSRVHeap()->GetCPUDescriptorHandleForHeapStart()
	);

	// ﾃﾞｨｽｸﾘﾌﾟﾀﾋｰﾌﾟのﾎﾟｲﾝﾀが初期値なら最初の定数ﾊﾞｯﾌｧﾋﾞｭｰ作成なので、ｱﾄﾞﾚｽ取得する。その後1ﾋﾞｭｰｱﾄﾞﾚｽ分加算する
	if (basicDescHeapHandle.ptr == 0)
	{
		basicDescHeapHandle = bufferHeapCreator->GetCBVSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	}
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	basicDescHeapHandle.ptr += inc;

	// ライトマップ計算用にもう一つ(7つめ)
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc4MultipassMatrix;
	cbvDesc4MultipassMatrix.BufferLocation = bufferHeapCreator->GetMatrixBuff4Multipass()->GetGPUVirtualAddress();
	cbvDesc4MultipassMatrix.SizeInBytes = bufferHeapCreator->GetMatrixBuff4Multipass()->GetDesc().Width;
	if (handle4SRVMultipass.ptr == 0)
	{
		handle4SRVMultipass = bufferHeapCreator->GetMultipassSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	}
	handle4SRVMultipass.ptr += inc * 6; // 7個目

	_dev->CreateConstantBufferView
	(
		&cbvDesc4MultipassMatrix,
		handle4SRVMultipass
	);
}

void ViewCreator::CreateCBV4ImguiPostSetting(ComPtr<ID3D12Device> _dev)
{
	// imgui PostSetting用に用意
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc4ImguiPostSetting;
	cbvDesc4ImguiPostSetting.BufferLocation = bufferHeapCreator->GetImguiPostSettingBuff()->GetGPUVirtualAddress();
	cbvDesc4ImguiPostSetting.SizeInBytes = bufferHeapCreator->GetImguiPostSettingBuff()->GetDesc().Width;

	auto handle4SRVMultipass = bufferHeapCreator->GetMultipassSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handle4SRVMultipass.ptr += inc * 13; // 14個目

	_dev->CreateConstantBufferView
	(
		&cbvDesc4ImguiPostSetting,
		handle4SRVMultipass
	);
}

void ViewCreator::CreateDSVWrapper(ComPtr<ID3D12Device> _dev)
{
	SetDSVDesc();
	auto handle = bufferHeapCreator->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();

	// 深度マップ用
	_dev->CreateDepthStencilView
	(
		bufferHeapCreator->GetDepthBuff().Get(),
		&dsvDesc,
		handle
	);

	// ライトマップ用
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	_dev->CreateDepthStencilView
	(
		bufferHeapCreator->GetLightMapBuff().Get(),
		&dsvDesc,
		handle
	);

	// for AO
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	_dev->CreateDepthStencilView
	(
		bufferHeapCreator->GetDepthBuff2().Get(),
		&dsvDesc,
		handle
	);
}

void ViewCreator::CreateCBVSRV4MateriallTextureSph(ComPtr<ID3D12Device> _dev)
{
	//  ﾃﾞｨｽｸﾘﾌﾟﾀﾋｰﾌﾟのﾎﾟｲﾝﾀが初期値なら最初の定数ﾊﾞｯﾌｧﾋﾞｭｰ作成なので、ｱﾄﾞﾚｽ取得する。その後1ﾋﾞｭｰｱﾄﾞﾚｽ分加算する
	if (basicDescHeapHandle.ptr == 0)
	{
		basicDescHeapHandle = bufferHeapCreator->GetCBVSRVHeap()->GetCPUDescriptorHandleForHeapStart();
	}
	auto inc = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	SetSRVDesc4MaterialAndTextureAndSph();
	SetCBVDesc4MaterialAndTextureAndSph();

	// 色テクスチャバッファ作成
	bufferHeapCreator->CreateTextureBuffers(_dev);
	// 白テクスチャバッファ
	whiteBuff = bufferHeapCreator->GetWhiteTextureBuff();
	// 黒テクスチャバッファ
	BlackBuff = bufferHeapCreator->GetBlackTextureBuff();
	// グレーグラデーション
	grayTexBuff = bufferHeapCreator->GetGrayTextureBuff();



	// 2つ目 深度マップ
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetDepthBuff().Get(),
		&depthSRVDesc,
		basicDescHeapHandle
	);

	basicDescHeapHandle.ptr += inc;

	// 3つ目 ライトマップ
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetLightMapBuff().Get(),
		&depthSRVDesc,
		basicDescHeapHandle
	);

	basicDescHeapHandle.ptr += inc;


	for (int i = 0; i < pmdMaterialInfo->materialNum; i++)
	{
		_dev->CreateConstantBufferView(&cbvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		basicDescHeapHandle.ptr += inc;

		cbvDesc4MaterialAndTextureAndSph.BufferLocation += bufferHeapCreator->GetMaterialBuffSize();

		// テクスチャ
		if (bufferHeapCreator->GetPMDTexReadBuff()[i] == nullptr)
		{
			srvDesc4MaterialAndTextureAndSph.Format = whiteBuff->GetDesc().Format;
			_dev->CreateShaderResourceView
			(whiteBuff.Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		else
		{
			srvDesc4MaterialAndTextureAndSph.Format = bufferHeapCreator->GetPMDTexReadBuff()[i]->GetDesc().Format;
			_dev->CreateShaderResourceView
			(bufferHeapCreator->GetPMDTexReadBuff()[i].Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		basicDescHeapHandle.ptr += inc;

		// sphファイル
		if (bufferHeapCreator->GetsphMappedBuff()[i] == nullptr)
		{
			srvDesc4MaterialAndTextureAndSph.Format = whiteBuff->GetDesc().Format;
			_dev->CreateShaderResourceView
			(whiteBuff.Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		else
		{
			srvDesc4MaterialAndTextureAndSph.Format = bufferHeapCreator->GetsphMappedBuff()[i]->GetDesc().Format;
			_dev->CreateShaderResourceView
			(bufferHeapCreator->GetsphMappedBuff()[i].Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		basicDescHeapHandle.ptr += inc;

		// spaファイル
		if (bufferHeapCreator->GetspaMappedBuff()[i] == nullptr)
		{
			srvDesc4MaterialAndTextureAndSph.Format = BlackBuff->GetDesc().Format;
			_dev->CreateShaderResourceView
			(BlackBuff.Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		else
		{
			srvDesc4MaterialAndTextureAndSph.Format = bufferHeapCreator->GetspaMappedBuff()[i]->GetDesc().Format;
			_dev->CreateShaderResourceView
			(bufferHeapCreator->GetspaMappedBuff()[i].Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		basicDescHeapHandle.ptr += inc;

		// トゥーンテクスチャファイル
		if (bufferHeapCreator->GetToonReadBuff()[i] == nullptr)
		{
			srvDesc4MaterialAndTextureAndSph.Format = grayTexBuff->GetDesc().Format;
			_dev->CreateShaderResourceView
			(grayTexBuff.Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		else
		{
			srvDesc4MaterialAndTextureAndSph.Format = bufferHeapCreator->GetToonReadBuff()[i]->GetDesc().Format;
			_dev->CreateShaderResourceView
			(bufferHeapCreator->GetToonReadBuff()[i].Get(), &srvDesc4MaterialAndTextureAndSph, basicDescHeapHandle);
		}

		basicDescHeapHandle.ptr += inc;
	}
}

void ViewCreator::CreateRTV4Multipasses(ComPtr<ID3D12Device> _dev)
{
	SetRTVDesc4Multipass();
	auto handle = bufferHeapCreator->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();

	// 一個目
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetMultipassBuff().Get(),
		&multipassRTVDesc,
		handle
	);

	//★★★後で試す
	//multipassRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // パスの二つ目はモデル描画なのでそれに合わせて変更する
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ニ個目
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetMultipassBuff2().Get(),
		&multipassRTVDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 3個目
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetMultipassBuff3().Get(),
		&multipassRTVDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// No.4 bloom-1
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetBloomBuff()[0].Get(),
		&multipassRTVDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// No.5 bloom-2
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetBloomBuff()[1].Get(),
		&multipassRTVDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// No.6 shrinkedModel
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetBloomBuff()[2].Get(),
		&multipassRTVDesc,
		handle
	);

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// No.7 AO
	multipassRTVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetAOBuff().Get(),
		&multipassRTVDesc,
		handle
	);

	multipassRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// No.8 imgui
	_dev->CreateRenderTargetView
	(
		bufferHeapCreator->GetImguiBuff().Get(),
		&multipassRTVDesc,
		handle
	);
}

void ViewCreator::CreateSRV4Multipasses(ComPtr<ID3D12Device> _dev)
{
	SetSRVDesc4Multipass();
	handle4SRVMultipass = bufferHeapCreator->GetMultipassSRVHeap()->GetCPUDescriptorHandleForHeapStart();

	// 1個目 ペラポリゴンのレンダリング結果
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetMultipassBuff().Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 2個目 モデルのレンダリング結果
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetMultipassBuff2().Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 3個目 ガウスぼかしウェイト
	effectCBVDesc.BufferLocation = bufferHeapCreator->GetGaussianBuff()->GetGPUVirtualAddress();
	effectCBVDesc.SizeInBytes = bufferHeapCreator->GetGaussianBuff()->GetDesc().Width;
	_dev->CreateConstantBufferView
	(
		&effectCBVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 4個目 割れノーマルマップ
	normalMapSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	normalMapSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	normalMapSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	normalMapSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetNormalMapReadBuff()[0].Get(),
		&normalMapSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 5個目 深度マップ
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetDepthBuff().Get(),
		&depthSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 6個目 ライトマップ
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthSRVDesc.Texture2D.MipLevels = 1;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetLightMapBuff().Get(),
		&depthSRVDesc,
		handle4SRVMultipass
	);


	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)*2;
	// 8個目 normal
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetMultipassBuff3().Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// No.9 bloom-1
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetBloomBuff()[0].Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// No.10 bloom-2
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetBloomBuff()[1].Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// No.11 shrinkedModel
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetBloomBuff()[2].Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// No.12 AO
	D3D12_SHADER_RESOURCE_VIEW_DESC srv4AO = multipassSRVDesc;
	srv4AO.Format = DXGI_FORMAT_R32_FLOAT;
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetAOBuff().Get(),
		&srv4AO,
		handle4SRVMultipass
	);

	handle4SRVMultipass.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// No.13 imgui
	_dev->CreateShaderResourceView
	(
		bufferHeapCreator->GetImguiBuff().Get(),
		&multipassSRVDesc,
		handle4SRVMultipass
	);
}

void ViewCreator::CreateVertexBufferView()
{
	vbView.BufferLocation = bufferHeapCreator->GetVertBuff()->GetGPUVirtualAddress();//バッファの仮想アドレス
	vbView.SizeInBytes = pmdMaterialInfo->vertices.size();//全バイト数
	vbView.StrideInBytes = pmdMaterialInfo->pmdvertex_size;//1頂点あたりのバイト数
}

void ViewCreator::CreateIndexBufferView()
{
	ibView.BufferLocation = bufferHeapCreator->GetIdxBuff()->GetGPUVirtualAddress();
	ibView.SizeInBytes = sizeof(pmdMaterialInfo->indices[0]) * pmdMaterialInfo->indices.size();
	ibView.Format = DXGI_FORMAT_R16_UINT;
}

void ViewCreator::SetCBVDesc4Matrix()
{
	cbvDesc.BufferLocation = bufferHeapCreator->GetMatrixBuff()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = bufferHeapCreator->GetMatrixBuff()->GetDesc().Width;
}

void ViewCreator::SetDSVDesc()
{
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
}

void ViewCreator::SetSRVDesc4MaterialAndTextureAndSph()
{
	srvDesc4MaterialAndTextureAndSph.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc4MaterialAndTextureAndSph.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc4MaterialAndTextureAndSph.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc4MaterialAndTextureAndSph.Texture2D.MipLevels = 1;
}

void ViewCreator::SetCBVDesc4MaterialAndTextureAndSph()
{
	cbvDesc4MaterialAndTextureAndSph.BufferLocation = bufferHeapCreator->GetMaterialBuff()->GetGPUVirtualAddress();
	cbvDesc4MaterialAndTextureAndSph.SizeInBytes = bufferHeapCreator->GetMaterialBuffSize();
}

void ViewCreator::SetRTVDesc4Multipass()
{
	multipassRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	multipassRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
}

void ViewCreator::SetSRVDesc4Multipass()
{
	multipassSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	multipassSRVDesc.Format = multipassRTVDesc.Format;
	multipassSRVDesc.Texture2D.MipLevels = 1;
	multipassSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

D3D12_VERTEX_BUFFER_VIEW* ViewCreator::GetVbView()
{
	return &vbView;
}

D3D12_INDEX_BUFFER_VIEW* ViewCreator::GetIbView()
{
	return &ibView;
}
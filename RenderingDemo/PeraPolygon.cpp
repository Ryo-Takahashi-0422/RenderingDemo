#include <stdafx.h>
#include <PeraPolygon.h>

PeraPolygon::~PeraPolygon()
{
	mappedPera = nullptr;
}

void PeraPolygon::CreatePeraView(ComPtr<ID3D12Device> _dev)
{
	auto peraHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto peraResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));

	// バッファー作成
	auto result = _dev->CreateCommittedResource
	(
		&peraHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&peraResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(peraBuff.ReleaseAndGetAddressOf())
	);
	
	// ビュー作成
	peraVBV.BufferLocation = peraBuff->GetGPUVirtualAddress();
	peraVBV.SizeInBytes = sizeof(pv);
	peraVBV.StrideInBytes = sizeof(PeraVertex);

	// マッピング
	peraBuff->Map(0, nullptr, (void**)&mappedPera);
	std::copy(std::begin(pv), std::end(pv), mappedPera);
	peraBuff->Unmap(0, nullptr);
}

D3D12_VERTEX_BUFFER_VIEW* PeraPolygon::GetVBView()
{
	return &peraVBV;
}
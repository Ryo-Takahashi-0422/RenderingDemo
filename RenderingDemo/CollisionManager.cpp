#include <stdafx.h>
#include <CollisionManager.h>

CollisionManager::CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers)
{
	dev = _dev;
	resourceManager = _resourceManagers;
	Init();
}

void CollisionManager::Init()
{
	auto vetmap1 = resourceManager[0]->GetIndiceAndVertexInfo();
	auto vetmap2 = resourceManager[1]->GetIndiceAndVertexInfo();

	for (int j = 0; j < resourceManager[0]->GetVertexTotalNum(); ++j)
	{
		input1.push_back(vetmap1.begin()->second.vertices[j].pos);
	}

	for (int j = 0; j < 4454; ++j)
	{
		input2.push_back(vetmap2.begin()->second.vertices[j].pos);
	}

	BoundingBox::CreateFromPoints(box1, input1.size(), input1.data(), (size_t)sizeof(input1.data()));
	BoundingBox::CreateFromPoints(box2, 4454, input2.data(), (size_t)sizeof(input2.data()));

	XMFLOAT3 temp1[8];
	XMFLOAT3 temp2[8];
	box1.GetCorners(temp1);
	box2.GetCorners(temp2);


	for (int i = 0; i < 8; ++i)
	{
		output1[i] = temp1[i];
		output2[i] = temp2[i];
	}

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(output1));


	// バッファー作成
	auto result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(boxBuff1.ReleaseAndGetAddressOf())
	);

	// ビュー作成
	boxVBV1.BufferLocation = boxBuff1->GetGPUVirtualAddress();
	boxVBV1.SizeInBytes = sizeof(output1);
	boxVBV1.StrideInBytes = sizeof(XMFLOAT3);

	// マッピング
	boxBuff1->Map(0, nullptr, (void**)&mappedBox1);
	std::copy(std::begin(output1), std::end(output1), mappedBox1);
	boxBuff1->Unmap(0, nullptr);


	resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(output2));
	result = dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(boxBuff2.ReleaseAndGetAddressOf())
	);

	// ビュー作成
	boxVBV1.BufferLocation = boxBuff2->GetGPUVirtualAddress();
	boxVBV1.SizeInBytes = sizeof(output2);
	boxVBV1.StrideInBytes = sizeof(XMFLOAT3);

	// マッピング
	boxBuff2->Map(0, nullptr, (void**)&mappedBox2);
	std::copy(std::begin(output2), std::end(output2), mappedBox2);
	boxBuff2->Unmap(0, nullptr);
}
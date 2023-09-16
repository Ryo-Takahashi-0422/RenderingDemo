#pragma once

struct BoundingBoxVertex
{
	XMFLOAT3 pos;
};

class CollisionManager
{
private:
	ComPtr<ID3D12Device> dev = nullptr;
	std::vector<ResourceManager*> resourceManager;
	void Init();

	std::vector<XMFLOAT3> input1;
	std::vector<XMFLOAT3> input2;

	BoundingBoxVertex bbv1[8];
	BoundingBoxVertex bbv2[8];

	XMFLOAT3 output1[8];
	XMFLOAT3 output2[8];

	XMFLOAT3* mappedBox1 = nullptr;
	XMFLOAT3* mappedBox2 = nullptr;
	ComPtr<ID3D12Resource> boxBuff1 = nullptr;
	ComPtr<ID3D12Resource> boxBuff2 = nullptr;
	D3D12_VERTEX_BUFFER_VIEW boxVBV1 = {};
	D3D12_VERTEX_BUFFER_VIEW boxVBV2 = {};

public:
	CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers);
	BoundingBox box1;
	BoundingBox box2;

	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV1() { return &boxVBV1; };
	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV2() { return &boxVBV2; };
};
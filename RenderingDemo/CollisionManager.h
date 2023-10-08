#pragma once

struct BoundingBoxVertex
{
	XMFLOAT3 pos;
};

//struct OBBVertices
//{
//	XMFLOAT3 pos[8];
//};

class CollisionManager
{
private:
	ComPtr<ID3D12Device> dev = nullptr;
	std::vector<ResourceManager*> resourceManager;
	void Init();

	BoundingOrientedBox box1;
	std::vector<BoundingOrientedBox> boxes;

	BoundingSphere bSphere;

	std::vector<XMFLOAT3> input1;
	std::vector<XMFLOAT3> input2;

	BoundingBoxVertex bbv1[8];
	BoundingBoxVertex bbv2[8];

	//XMFLOAT3 output1[8];
	std::vector<XMFLOAT3> output1;
	XMFLOAT3 output2[8];
	XMFLOAT3 output3[26];

	XMFLOAT3* mappedBox1 = nullptr;
	XMFLOAT3* mappedBox2 = nullptr;
	ComPtr<ID3D12Resource> boxBuff1 = nullptr;
	ComPtr<ID3D12Resource> boxBuff2 = nullptr;
	D3D12_VERTEX_BUFFER_VIEW boxVBV1 = {};
	D3D12_VERTEX_BUFFER_VIEW boxVBV2 = {};

	void CreateSpherePoints(const XMFLOAT3& pt, float Radius);

public:
	CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers);

	/*BoundingOrientedBox*/std::vector<BoundingOrientedBox> GetBoundingBox1() { return boxes; };
	/*BoundingOrientedBox*/BoundingOrientedBox* GetBoundingBox1Pointer() { return boxes.data(); };

	BoundingSphere GetBoundingSphere() { return bSphere; };
	BoundingSphere* GetBoundingSpherePointer() { return &bSphere; };

	XMFLOAT3* GetMappedBoxPos() { return mappedBox2; };

	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV1() { return &boxVBV1; };
	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV2() { return &boxVBV2; };

	void MoveCharacterBoundingBox(double speed, XMMATRIX charaDirection);
};
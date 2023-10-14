#pragma once

struct BoundingBoxVertex
{
	XMFLOAT3 pos;
};

struct OBBVertices
{
	XMFLOAT3 pos[8];
};

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

	std::vector<OBBVertices> oBBVertices;
	XMFLOAT3 output2[8];
	XMFLOAT3 output3[26];

	std::vector<XMFLOAT3*> mappedOBBs; // 各OBBの8頂点のマッピング先
	XMFLOAT3* mappedBox2 = nullptr;
	std::vector<ComPtr<ID3D12Resource>> boxBuffs; // ★

	ComPtr<ID3D12Resource> boxBuff2 = nullptr;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> boxVBVs; // ★
	std::map<int, std::vector<int>> oBBIndices;
	std::vector<D3D12_INDEX_BUFFER_VIEW> boxIBVs; // 各OBBのインデクスバッファビュー
	std::vector<ComPtr<ID3D12Resource>> boxIbBuffs; // インデックスバッファのリソース
	std::vector<unsigned int*> mappedIdx;

	D3D12_VERTEX_BUFFER_VIEW boxVBV2 = {};

	BoundingOrientedBox collidedOBB;
	void CreateSpherePoints(const XMFLOAT3& pt, float Radius);
	XMFLOAT3 CalculateForthPoint(std::vector<XMFLOAT3> storedPoints, XMFLOAT3 boxPoints[8]);
	std::pair<XMVECTOR, XMVECTOR> CalcurateNormalAndSlideVector(std::vector<XMFLOAT3> points, XMFLOAT3 boxCenter);
	void StoreIndiceOfOBB(std::map<int, std::vector<std::pair<float, int>>> res, int loopCnt, int index);

public:
	CollisionManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers);

	std::vector<BoundingOrientedBox> GetBoundingBox1() { return boxes; };
	BoundingOrientedBox* GetBoundingBox1Pointer() { return &boxes[2]; };

	BoundingSphere GetBoundingSphere() { return bSphere; };
	BoundingSphere* GetBoundingSpherePointer() { return &bSphere; };

	XMFLOAT3* GetMappedBoxPos() { return mappedBox2; };

	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV2() { return &boxVBV2; };

	void MoveCharacterBoundingBox(double speed, XMMATRIX charaDirection);
	bool OBBCollisionCheck();
	void OBBTransrationWithCollision(float forwardSpeed, XMMATRIX characterDirection, int fbxIndex);
	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBVs(int index) { return &boxVBVs[index]; }; // ★
	D3D12_INDEX_BUFFER_VIEW* GetBoxIBVs(int index) { return &boxIBVs[index]; }; // ★
	int GetOBBNum() { return oBBVertices.size(); };
};
#pragma once

struct OBBVertex
{
	XMFLOAT3 pos;
};

struct OBBAllVertices
{
	XMFLOAT3 pos[8];
};

class OBBManager
{
private:
	HRESULT Init();
	ComPtr<ID3D12Device> dev = nullptr;
	std::vector<ResourceManager*> resourceManager;

	PeraLayout* layout = nullptr;
	ColliderGraphicsPipelineSetting* colliderGraphicsPipelineSetting = nullptr;
	CollisionRootSignature* collisionRootSignature = nullptr;
	SettingShaderCompile* collisionShaderCompile = nullptr;

	ComPtr<ID3D10Blob> _vsCollisionBlob = nullptr; // コライダー描画用
	ComPtr<ID3D10Blob> _psCollisionBlob = nullptr; // コライダー描画用


	void CreateInfo();

	BoundingOrientedBox box1;
	std::vector<BoundingOrientedBox> boxes;

	BoundingSphere bSphere;

	std::vector<XMFLOAT3> input1;
	std::vector<XMFLOAT3> input2;

	OBBVertex bbv1[8];
	OBBVertex bbv2[8];

	std::vector<OBBAllVertices> oBBVertices;
	XMFLOAT3 output2[8];
	XMFLOAT3 output3[26]; // ｷｬﾗｸﾀｰｽﾌｨｱｺﾗｲﾀﾞｰの頂点
	//ComPtr<ID3D12Resource> sphereIbBuff; // ｷｬﾗｸﾀｰｽﾌｨｱｺﾗｲﾀﾞｰｲﾝﾃﾞｯｸｽﾊﾞｯﾌｧのリソース
	//std::vector<int> sphereColliderIndices;
	//D3D12_INDEX_BUFFER_VIEW sphereIBV;
	//unsigned int* mappedSphereIdx;

	std::vector<XMFLOAT3*> mappedOBBs; // 各OBBの8頂点のマッピング先
	XMFLOAT3* mappedBox2 = nullptr;
	std::vector<ComPtr<ID3D12Resource>> boxBuffs; // ★

	ComPtr<ID3D12Resource> boxBuff2 = nullptr;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> boxVBVs; // ★
	std::map<int, std::vector<int>> oBBIndices;
	std::vector<D3D12_INDEX_BUFFER_VIEW> boxIBVs; // 各OBBのインデクスバッファビュー
	std::vector<ComPtr<ID3D12Resource>> boxIbBuffs; // OBB用インデックスバッファのリソース
	std::vector<unsigned int*> mappedIdx;

	D3D12_VERTEX_BUFFER_VIEW boxVBV2 = {};

	BoundingOrientedBox collidedOBB;
	//void CreateSpherePoints(const XMFLOAT3& pt, float Radius);
	std::pair<XMVECTOR, XMVECTOR> CalcurateNormalAndSlideVector(std::vector<XMFLOAT3> points, XMFLOAT3 boxCenter);
	void StoreIndiceOfOBB(std::map<int, std::vector<std::pair<float, int>>> res, int loopCnt, int index);

	static const size_t CORNER_COUNT = 8;

	HRESULT CreateMatrixHeap();
	HRESULT CreateMatrixResources();
	void CreateMatrixView();
	HRESULT MappingMatrix();
	ComPtr<ID3D12Resource> matrixBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12DescriptorHeap> matrixHeap = nullptr; // RTV用ディスクリプタヒープ
	struct CollisionMatrix
	{
		XMMATRIX rotation;
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	CollisionMatrix* mappedMatrix = nullptr;
	XMFLOAT3 charaPos;

public:
	OBBManager(ComPtr<ID3D12Device> _dev, std::vector<ResourceManager*> _resourceManagers);

	std::vector<BoundingOrientedBox> GetBoundingBox1() { return boxes; };
	BoundingOrientedBox* GetBoundingBox1Pointer() { return &boxes[2]; };

	BoundingSphere GetBoundingSphere() { return bSphere; };
	BoundingSphere* GetBoundingSpherePointer() { return &bSphere; };

	XMFLOAT3* GetMappedBoxPos() { return mappedBox2; };

	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBV2() { return &boxVBV2; };


	D3D12_VERTEX_BUFFER_VIEW* GetBoxVBVs(int index) { return &boxVBVs[index]; }; // ★
	D3D12_INDEX_BUFFER_VIEW* GetBoxIBVs(int index) { return &boxIBVs[index]; }; // ★

	int GetOBBNum() { return oBBVertices.size(); };

	ComPtr<ID3D12DescriptorHeap> GetMatrixHeap() { return matrixHeap; };
	void SetMatrix(XMMATRIX _world, XMMATRIX _view, XMMATRIX _proj);
	void SetCharaPos(XMFLOAT3 _charaPos);

	void Execution(ID3D12GraphicsCommandList* _cmdList);
};
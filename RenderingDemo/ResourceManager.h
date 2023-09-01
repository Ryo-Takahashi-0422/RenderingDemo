#pragma once

//シェーダー側に渡す基本的な行列データ
struct FBXSceneMatrix
{
	XMMATRIX world; // world matrix
	XMMATRIX view; // view matrix
	XMMATRIX proj; // projection matri
	//XMMATRIX lightCamera; // view from light(view * projection)
	//XMMATRIX shadow; // shadow matrix
	//XMFLOAT3 eye; // position of camera
	//XMMATRIX invProj; // inverse projection matrix
	//XMMATRIX invView; // inverted view matrix
	//XMMATRIX bones[256]; // pmd bone matrix

	//float lightVec[3]; // vector of light from imgui
	//bool isSelfShadow; // Self Shadow on/off
};

class ResourceManager
{
private:
	ComPtr<ID3D12Device> _dev = nullptr;
	FBXInfoManager* _fbxInfoManager = nullptr;
	PrepareRenderingWindow* _prepareRenderingWindow = nullptr;

	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ

	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12Resource> depthBuff = nullptr; // depth buffer
	ComPtr<ID3D12Resource> renderingBuff = nullptr; // rendering model buffer
	ComPtr<ID3D12Resource> matrixBuff = nullptr; // matrix buffer

	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertexビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Indexビュー

	/*std::vector<float>*/std::vector<FBXVertex> verticesPosContainer;
	/*std::vector<int>*/std::vector<unsigned int> indexContainer;
	FBXVertex*/*unsigned char**/ mappedVertPos = nullptr;
	unsigned int* mappedIdx = nullptr;
	int vertexTotalNum; // vertex total num
	int indexNum; // index total num

	FBXSceneMatrix* mappedMatrix = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;

	HRESULT CreateRTV();
	HRESULT CreateAndMapMatrix();

	void ClearReference();

public:
	ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, PrepareRenderingWindow* prepareRederingWindow);
	HRESULT Init();
	ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return rtvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetSRVHeap() { return srvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return dsvHeap; };
	ComPtr<ID3D12Resource> GetRenderingBuff() { return renderingBuff; };
	D3D12_VERTEX_BUFFER_VIEW* GetVbView() { return &vbView; };
	D3D12_INDEX_BUFFER_VIEW* GetIbView() { return &ibView; };
	int GetVertexTotalNum() { return vertexTotalNum; };
	int GetIndexTotalNum() { return indexNum; };

	FBXSceneMatrix* GetMappedMatrix() { return mappedMatrix; };
};


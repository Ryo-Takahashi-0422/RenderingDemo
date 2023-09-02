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
	TextureLoader* textureLoader = nullptr;

	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ

	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12Resource> depthBuff = nullptr; // depth buffer
	ComPtr<ID3D12Resource> renderingBuff = nullptr; // rendering model buffer
	ComPtr<ID3D12Resource> matrixBuff = nullptr; // matrix buffer
	ComPtr<ID3D12Resource> materialParamBuff0 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff1 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff2 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff3 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff4 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff5 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff6 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff7 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff8 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff9 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff10 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff11 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff12 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff13 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff14 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff15 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff16 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff17 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff18 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff19 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff20 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff21 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff22 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff23 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff24 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff25 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff26 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff27 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff28 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff29 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff30 = nullptr; // Phong Material Parameter
	ComPtr<ID3D12Resource> materialParamBuff31 = nullptr; // Phong Material Parameter

	// able to contain 32 materials
	std::vector<ComPtr<ID3D12Resource>> materialParamBuffContainer =
	{ materialParamBuff0 , materialParamBuff1, materialParamBuff2 ,materialParamBuff3 ,materialParamBuff4 ,materialParamBuff5 ,materialParamBuff6 ,materialParamBuff7 ,materialParamBuff8,materialParamBuff9,materialParamBuff10,
	  materialParamBuff11, materialParamBuff12, materialParamBuff13, materialParamBuff14, materialParamBuff15, materialParamBuff16, materialParamBuff17, materialParamBuff18, materialParamBuff19, materialParamBuff20,
	  materialParamBuff21, materialParamBuff22, materialParamBuff23, materialParamBuff24, materialParamBuff25, materialParamBuff26, materialParamBuff27, materialParamBuff28, materialParamBuff29, materialParamBuff30, materialParamBuff31
	};

	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertexビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Indexビュー

	std::vector<FBXVertex> verticesPosContainer;
	std::vector<unsigned int> indexContainer;
	FBXVertex* mappedVertPos = nullptr;
	unsigned int* mappedIdx = nullptr;

	int vertexTotalNum; // vertex total num
	int indexNum; // index total num

	FBXSceneMatrix* mappedMatrix = nullptr;
	PhongInfo* mappedPhong0 = nullptr;
	PhongInfo* mappedPhong1 = nullptr;
	PhongInfo* mappedPhong2 = nullptr;
	PhongInfo* mappedPhong3 = nullptr;
	PhongInfo* mappedPhong4 = nullptr;
	PhongInfo* mappedPhong5 = nullptr;
	PhongInfo* mappedPhong6 = nullptr;
	PhongInfo* mappedPhong7 = nullptr;
	PhongInfo* mappedPhong8 = nullptr;
	PhongInfo* mappedPhong9 = nullptr;
	PhongInfo* mappedPhong10 = nullptr;
	PhongInfo* mappedPhong11 = nullptr;
	PhongInfo* mappedPhong12 = nullptr;
	PhongInfo* mappedPhong13 = nullptr;
	PhongInfo* mappedPhong14 = nullptr;
	PhongInfo* mappedPhong15 = nullptr;
	PhongInfo* mappedPhong16 = nullptr;
	PhongInfo* mappedPhong17 = nullptr;
	PhongInfo* mappedPhong18 = nullptr;
	PhongInfo* mappedPhong19 = nullptr;
	PhongInfo* mappedPhong20 = nullptr;
	PhongInfo* mappedPhong21 = nullptr;
	PhongInfo* mappedPhong22 = nullptr;
	PhongInfo* mappedPhong23 = nullptr;
	PhongInfo* mappedPhong24 = nullptr;
	PhongInfo* mappedPhong25 = nullptr;
	PhongInfo* mappedPhong26 = nullptr;
	PhongInfo* mappedPhong27 = nullptr;
	PhongInfo* mappedPhong28 = nullptr;
	PhongInfo* mappedPhong29 = nullptr;
	PhongInfo* mappedPhong30 = nullptr;
	PhongInfo* mappedPhong31 = nullptr;
	// able to contain 32 materials
	std::vector<PhongInfo*> mappedPhoneContainer =
	{
		mappedPhong0, mappedPhong1, mappedPhong2, mappedPhong3, mappedPhong4, mappedPhong5, mappedPhong6, mappedPhong7, mappedPhong8, mappedPhong9,
		mappedPhong10, mappedPhong11, mappedPhong12, mappedPhong13, mappedPhong14, mappedPhong15, mappedPhong16, mappedPhong17, mappedPhong18, mappedPhong19,
		mappedPhong20, mappedPhong21, mappedPhong22, mappedPhong23, mappedPhong24, mappedPhong25, mappedPhong26, mappedPhong27, mappedPhong28, mappedPhong29, mappedPhong30, mappedPhong31
	};

	D3D12_CPU_DESCRIPTOR_HANDLE handle;

	HRESULT CreateRTV();
	HRESULT CreateAndMapMatrix();

	void CreateUploadAndReadBuff4Texture(std::string strModelPath, std::string fileType);
	std::vector<ComPtr<ID3D12Resource>> textureUploadBuff; // ノーマルマップ用アップロードバッファー
	std::vector<ComPtr<ID3D12Resource>> textureReadBuff; // ノーマルマップ用リードバッファー
	std::vector<DirectX::TexMetadata*> textureMetaData;
	std::vector<DirectX::Image*> textureImg;

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
	std::vector<PhongInfo*> GetMappedPhong() { return mappedPhoneContainer; };
};


#pragma once

//シェーダー側に渡す基本的な行列データ
struct FBXSceneMatrix
{
	XMMATRIX charaRot;
	XMMATRIX world; // world matrix
	XMMATRIX view; // view matrix
	XMMATRIX proj; // projection matri
	XMMATRIX oProj;
	XMMATRIX bones[256]; // pmd bone matrix // index number is equal with bones index number
	//XMMATRIX invProj;
	XMMATRIX rotation;
	XMMATRIX shadowPosMatrix;
	XMMATRIX shadowPosInvMatrix;
	XMMATRIX shadowView;
	XMFLOAT3 eyePos;
	float pad0;
	XMFLOAT3 sunDIr;
	float pad1;
	XMFLOAT3 charaPos;
	bool sponzaDraw;
	bool dummy0;
	bool dummy1;
	bool dummy2;
	bool airDraw;
	bool dummy3;
	bool dummy4;
	bool dummy5;
	bool brdfSpecularDraw;
	bool dummy6;
	bool dummy7;
	bool dummy8;
	float pad2;
	float pad3;
	XMFLOAT3 plPos1;
	float pad4;
	XMFLOAT3 plPos2;
};

class ResourceManager
{
private:
	ComPtr<ID3D12Device> _dev = nullptr;
	FBXInfoManager* _fbxInfoManager = nullptr;
	PrepareRenderingWindow* _prepareRenderingWindow = nullptr;
	TextureLoader* textureLoader = nullptr;
	Camera* m_Camera = nullptr;

	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ

	ComPtr<ID3D12Resource> vertBuff = nullptr; // vertex pos mapped buffer
	ComPtr<ID3D12Resource> idxBuff = nullptr; // index mapped buffer
	ComPtr<ID3D12Resource> depthBuff = nullptr; // depth buffer
	ComPtr<ID3D12Resource> depthBuff2 = nullptr; // depth buffer
	ComPtr<ID3D12Resource> skyBuffer = nullptr; // Sky描画結果
	ComPtr<ID3D12Resource> imguiBuffer = nullptr; // ImGui描画結果
	ComPtr<ID3D12Resource> sunBuffer = nullptr; // sun描画結果
	ComPtr<ID3D12Resource> airBuffer = nullptr; // air描画結果
	ComPtr<ID3D12Resource> vsmBuffer = nullptr; // vsm描画結果
	ComPtr<ID3D12Resource> shadowmapBuffer = nullptr; // シャドウマップ描画結果
	ComPtr<ID3D12Resource> renderingBuff = nullptr; // rendering model buffer
	ComPtr<ID3D12Resource> renderingBuff2 = nullptr; // rendering model buffer
	ComPtr<ID3D12Resource> normalMapBuff = nullptr; // 法線マップ
	ComPtr<ID3D12Resource> normalMapBuff2 = nullptr; // 法線マップ
	ComPtr<ID3D12Resource> matrixBuff = nullptr; // matrix buffer
	
	const int mappingSize = 32; // mapping size
	std::vector<ComPtr<ID3D12Resource>> materialParamBuffContainer; // resource of mappedPhoneContainer[...]
	std::vector<PhongInfo*> mappedPhoneContainer; // mapped phong infos

	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertexビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Indexビュー

	std::vector<FBXVertex> verticesPosContainer;
	std::vector<unsigned int> indexContainer;
	FBXVertex* mappedVertPos = nullptr;
	unsigned int* mappedIdx = nullptr;

	int vertexTotalNum; // vertex total num
	int indexNum; // index total num

	FBXSceneMatrix* mappedMatrix = nullptr;
	

	D3D12_CPU_DESCRIPTOR_HANDLE handle;

	HRESULT CreateRTV();
	HRESULT CreateAndMapResources(size_t textureNum);

	void CreateUploadAndReadBuff4Texture(std::string texturePath, int iterationNum);
	void MappingTextureToUploadBuff(int count);

	std::vector<uint8_t*> mappedImgContainer; // mapped Img

	std::vector<ComPtr<ID3D12Resource>> textureUploadBuff; // ノーマルマップ用アップロードバッファー
	std::vector<ComPtr<ID3D12Resource>> textureReadBuff; // ノーマルマップ用リードバッファー
	std::vector<DirectX::TexMetadata*> textureMetaData;
	std::vector<DirectX::Image*> textureImg;
	std::vector<unsigned int> textureImgPixelValue;
	ScratchImage scratchImg = {};

	DWORD _startTime; // アニメーション開始時のミリ秒
	DWORD elapsedTime; // 経過ミリ秒
	unsigned int frameNo; // 現在のフレームNo

	XMMATRIX invIdentify = XMMatrixIdentity();
	std::vector<XMMATRIX> invBonesInitialPostureMatrixMap;

	std::vector<std::pair<std::string, VertexInfo>> vertMap;
	std::vector<std::pair<std::string, VertexInfo>> meshVertexInfos;
	std::vector<std::pair<std::string, PhongInfo>> phongInfos;
	std::vector<std::pair<std::string, std::string>> materialAndTexturePath;
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> animationNameAndBoneNameWithTranslationMatrix;
	bool isAnimationModel = false;
	XMFLOAT3 localRotationFloat; // FBXManagerより取得したfbxモデルのxyzローカル回転
	//std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> localPosAndRotOfMesh; // メッシュ毎のローカル座標・回転
	std::vector<XMMATRIX> localMatrix; // メッシュ毎のローカル座標・回転
	std::map<std::string, XMMATRIX> localMatrix4OBB; // OBB毎のローカル座標・回転
	std::vector<std::pair<std::string, VertexInfo>> vertexListOfOBB;

	int descriptorNum;

public:
	ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager, PrepareRenderingWindow* prepareRederingWindow);
	~ResourceManager();
	HRESULT Init(Camera* _camera);
	ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return rtvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetSRVHeap() { return srvHeap; };
	ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return dsvHeap; };
	ComPtr<ID3D12Resource> GetRenderingBuff() { return renderingBuff; }; // カラーレンダリング結果　スレッド1
	ComPtr<ID3D12Resource> GetRenderingBuff2() { return renderingBuff2; }; // カラーレンダリング結果　スレッド2
	ComPtr<ID3D12Resource> GetNormalRenderingBuff() { return normalMapBuff; }; // 法線画像レンダリング結果　スレッド1
	ComPtr<ID3D12Resource> GetNormalRenderingBuff2() { return normalMapBuff2; }; // 法線画像レンダリング結果　スレッド2
	ComPtr<ID3D12Resource> GetDepthBuff() { return depthBuff; };
	ComPtr<ID3D12Resource> GetDepthBuff2() { return depthBuff2; };
	ComPtr<ID3D12Resource> GetAirBuff() { return airBuffer; };
	std::vector<ComPtr<ID3D12Resource>> GetTextureUploadBuff() { return textureUploadBuff; };
	std::vector<ComPtr<ID3D12Resource>> GetTextureReadBuff() { return textureReadBuff; };
	std::vector<DirectX::TexMetadata*> GetTextureMetaData() { return textureMetaData; };
	std::vector<DirectX::Image*> GetTextureImg() { return textureImg; };


	D3D12_VERTEX_BUFFER_VIEW* GetVbView() { return &vbView; };
	D3D12_INDEX_BUFFER_VIEW* GetIbView() { return &ibView; };
	int GetVertexTotalNum() { return vertexTotalNum; };
	int GetIndexTotalNum() { return indexNum; };

	FBXSceneMatrix* GetMappedMatrix() { return mappedMatrix; };
	std::vector<PhongInfo*> GetMappedPhong() { return mappedPhoneContainer; };

	void PlayAnimation();
	void MotionUpdate(std::string motionName, unsigned int maxFrameNum);
	unsigned int GetFrameNo() { return frameNo; };

	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfo() { return vertMap; };
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfoOfRenderingMesh() { return meshVertexInfos; };
	std::vector<std::pair<std::string, PhongInfo>> GetPhongMaterialParamertInfo() { return phongInfos; };
	std::vector<std::pair<std::string, std::string>> GetMaterialAndTexturePath() { return materialAndTexturePath; };
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> GetAnimationNameAndBoneNameWithTranslationMatrix() { return animationNameAndBoneNameWithTranslationMatrix; };

	bool GetIsAnimationModel() { return isAnimationModel; };
	std::map<std::string, XMMATRIX> GetLocalMatrixOfOBB() { return localMatrix4OBB; }; // メッシュ毎のローカル座標・回転を取得可能

	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfoOfOBB() { return vertexListOfOBB; };

	void ClearReference();
	void SetProjMatrix(XMMATRIX _proj);
	void SetSunResourceAndCreateView(ComPtr<ID3D12Resource> _sunResource);
	void SetSkyResourceAndCreateView(ComPtr<ID3D12Resource> _skyResource);
	void SetImGuiResourceAndCreateView(ComPtr<ID3D12Resource> _imguiResource);
	void SetAirResourceAndCreateView(ComPtr<ID3D12Resource> _airResource);
	void SetVSMResourceAndCreateView(ComPtr<ID3D12Resource> _vsmResource);
	void SetShadowResourceAndCreateView(ComPtr<ID3D12Resource> _shadowResource);
	int GetDescriptorNum() { return descriptorNum; };
	FBXSceneMatrix* GetMappedMatrixPointer() { return mappedMatrix; };
	void SetSceneInfo(XMMATRIX _shadowPosMatrix, XMMATRIX _shadowPosInvMatrix, XMMATRIX _shadowView, XMFLOAT3 _eyePos, XMFLOAT3 _sunDIr);

	void SetAirDraw(bool _isDraw);
	void SetBRDFSpecularDraw(bool _isDraw);
	void SetLightPos1(XMFLOAT3 pos);
	void SetLightPos2(XMFLOAT3 pos);
};


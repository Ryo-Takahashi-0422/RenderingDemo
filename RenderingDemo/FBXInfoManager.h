#pragma once

// Lambert Material type
struct LambertInfo
{
	float diffuse[3];
	float ambient[3];
	float emissive[3];
	float bump[3];
	float alpha;
};

// Phong Material Info
struct PhongInfo
{
	float diffuse[3];
	float ambient[3];
	float emissive[3];
	float bump[3];
	float specular[3];
	float reflection[3];
	float shineness;
};

struct FBXVertex
{
	float pos[3];
	float normal[3];
	float uv[2];
};

struct VertexInfo
{
	std::vector<FBXVertex> vertices;
	std::vector<unsigned short> indices;
};

class FBXInfoManager
{
private:
	int indiceCnt = 0;

	FbxManager* manager = nullptr;
	FbxScene* scene = nullptr;
	void printSpace(int count);

	void enumNodeNamesAndAttributes(FbxNode* node, int indent, const std::string& filePath/*, VertexInfo* vertexInfo*/);

	std::vector<FbxMesh*> fbxMeshes;
	int meshCnt = 0;
	FbxMesh* fbxMesh = nullptr;

	int vertNum = 0;
	std::vector<int> indiceVec; // 右手系インデクス
	std::map<std::string, std::vector<int>> fixedIndiceVec; // 左手系インデクス]DirectX用

	std::map<std::string, VertexInfo> finalInfo;
	static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);

public:
	int Init();
	int GetVertNum() { return vertNum; };
	int GetIndexNum() { return indiceVec.size(); };
	std::map<std::string, std::vector<int>> GetIndiceContainer() { return fixedIndiceVec; };
	std::map<std::string, VertexInfo> GetIndiceAndVertexInfo() { return finalInfo; };
};
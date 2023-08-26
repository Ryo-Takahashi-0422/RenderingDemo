#pragma once

struct FBXPolygonInfo
{
	int PolygonNum; // eMesh info 
	int PolygonVertexNum; // eMesh info 
	int* IndexAry; // eMesh info 
	int vertexNum; // vertex num
	FbxVector4* controlPoints;
};

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

//頂点構造体(PMDと同様) これをFBXファイルから読み取っていく
struct VertexInfo
{   //32byte(temporary)
	float pos[3]; // x, y, z // 座標 12byte
	float normal_vec[3]; // nx, ny, nz // 法線ベクトル 12byte
	float uv[2]; // u, v // UV座標 // MMDは頂点UV 8byte
	//std::string uvname; // uvセット名
	//LambertInfo lamberInfo;
	//PhongInfo phongInfo;
	//unsigned short bone_num[2]; // ボーン番号1、番号2 // モデル変形(頂点移動)時に影響 4byte
	//unsigned char bone_weight; // ボーン1に与える影響度 // min:0 max:100 // ボーン2への影響度は、(100 - bone_weight) 1byte
	//unsigned char edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合 1byte
};

class FBXInfoManager
{
private:
	FbxManager* manager = nullptr;
	void printSpace(int count);
	void enumNodeNamesAndAttributes(FbxNode* node, int indent);

	std::map<std::string, std::map<std::string, FBXPolygonInfo>> fbxModelInfo; // model name & Mesh name, FBXPolygonInfo
	std::map<std::string, FBXPolygonInfo> fbxPolygonMap; // Mesh name & FBXMesh
	std::vector<FbxMesh*> fbxMeshes;
	int meshCnt = 0;
	FbxMesh* fbxMesh = nullptr;

	int totalPolygonVertexNum = 0;
	std::vector<FbxDouble> vertexControlPointsXYZ; // 全頂点の座標
	std::vector<FbxDouble> vertexNormalXYZ; // 全頂点の法線座標

	std::map<std::string, std::vector<VertexInfo>> m_VertexInfo;
	std::vector<unsigned char> vertices{}; // VertexInfoを1頂点ごとに1byteで順番に並べたデータ

	int vertNum = 0;
	std::vector<int> indiceVec;

public:
	int Init();
	int GetVertNum() { return vertNum; };
	int GetIndexNum() { return indiceVec.size(); };
};
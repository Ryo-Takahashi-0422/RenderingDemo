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

////���_�\����(PMD�Ɠ��l) �����FBX�t�@�C������ǂݎ���Ă���
//struct VertexInfo
//{   //32byte(temporary)
//	float pos[3]; // x, y, z // ���W 12byte
//	float normal_vec[3]; // nx, ny, nz // �@���x�N�g�� 12byte
//	float uv[2]; // u, v // UV���W // MMD�͒��_UV 8byte
//	//std::string uvname; // uv�Z�b�g��
//	//LambertInfo lamberInfo;
//	//PhongInfo phongInfo;
//	//unsigned short bone_num[2]; // �{�[���ԍ�1�A�ԍ�2 // ���f���ό`(���_�ړ�)���ɉe�� 4byte
//	//unsigned char bone_weight; // �{�[��1�ɗ^����e���x // min:0 max:100 // �{�[��2�ւ̉e���x�́A(100 - bone_weight) 1byte
//	//unsigned char edge_flag; // 0:�ʏ�A1:�G�b�W���� // �G�b�W(�֊s)���L���̏ꍇ 1byte
//};

class FBXInfoManager
{


private:
	int indiceCnt = 0;

	FbxManager* manager = nullptr;
	FbxScene* scene = nullptr;
	void printSpace(int count);

	void enumNodeNamesAndAttributes(FbxNode* node, int indent, const std::string& filePath, VertexInfo* vertexInfo);

	std::map<std::string, std::map<std::string, FBXPolygonInfo>> fbxModelInfo; // model name & Mesh name, FBXPolygonInfo
	std::map<std::string, FBXPolygonInfo> fbxPolygonMap; // Mesh name & FBXMesh
	std::vector<FbxMesh*> fbxMeshes;
	int meshCnt = 0;
	FbxMesh* fbxMesh = nullptr;

	int totalPolygonVertexNum = 0;
	std::vector<FbxDouble> vertexControlPointsXYZ; // �S���_�̍��W
	std::vector<FbxDouble> vertexNormalXYZ; // �S���_�̖@�����W

	//std::map<std::string, std::vector<VertexInfo>> m_VertexInfo;
	//std::vector<unsigned char> vertices{}; // VertexInfo��1���_���Ƃ�1byte�ŏ��Ԃɕ��ׂ��f�[�^

	int vertNum = 0;
	std::vector<int> indiceVec; // �E��n�C���f�N�X
	std::map<std::string, std::vector<int>> fixedIndiceVec; // ����n�C���f�N�X]DirectX�p
	std::vector<int> indiceVecOfVec; // ��UV�̃C���f�N�X�@indiceVec�Ɣ�r���Ă݂�

	std::map<std::string, std::map<int, std::array<float, 3>>> indexAndVertexPosByMeshName; // index�����ꂽ���_���@����Ɋ�Â��`�悷�ׂ�
	std::map<std::string, std::map<int, std::array<float, 2>>> indexAndUVByMeshName; // index�����ꂽUV���@����Ɋ�Â��`�悷�ׂ�



	VertexInfo fbxVertexInfo = {};
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
	//std::map<std::string, std::vector<VertexInfo>> GetVertexMap() { return m_VertexInfo; };
	std::map<std::string, std::vector<int>> GetIndiceContainer() { return fixedIndiceVec; };

	/*std::map<std::string, std::map<int, std::array<float, 3>>> GetIndexAndVertexPosByMeshName() { return indexAndVertexPosByMeshName; };*/
	//std::vector<int> GetTestIndiceVec() { return indiceVec; };
	std::map<std::string, VertexInfo> GetIndiceAndVertexInfo() { return finalInfo; };





	static bool Load(const std::string& filePath, VertexInfo* vertexInfo);
};
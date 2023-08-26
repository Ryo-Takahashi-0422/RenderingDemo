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

//���_�\����(PMD�Ɠ��l) �����FBX�t�@�C������ǂݎ���Ă���
struct VertexInfo
{   //32byte(temporary)
	float pos[3]; // x, y, z // ���W 12byte
	float normal_vec[3]; // nx, ny, nz // �@���x�N�g�� 12byte
	float uv[2]; // u, v // UV���W // MMD�͒��_UV 8byte
	//std::string uvname; // uv�Z�b�g��
	//LambertInfo lamberInfo;
	//PhongInfo phongInfo;
	//unsigned short bone_num[2]; // �{�[���ԍ�1�A�ԍ�2 // ���f���ό`(���_�ړ�)���ɉe�� 4byte
	//unsigned char bone_weight; // �{�[��1�ɗ^����e���x // min:0 max:100 // �{�[��2�ւ̉e���x�́A(100 - bone_weight) 1byte
	//unsigned char edge_flag; // 0:�ʏ�A1:�G�b�W���� // �G�b�W(�֊s)���L���̏ꍇ 1byte
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
	std::vector<FbxDouble> vertexControlPointsXYZ; // �S���_�̍��W
	std::vector<FbxDouble> vertexNormalXYZ; // �S���_�̖@�����W

	std::map<std::string, std::vector<VertexInfo>> m_VertexInfo;
	std::vector<unsigned char> vertices{}; // VertexInfo��1���_���Ƃ�1byte�ŏ��Ԃɕ��ׂ��f�[�^

	int vertNum = 0;
	std::vector<int> indiceVec;

public:
	int Init();
	int GetVertNum() { return vertNum; };
	int GetIndexNum() { return indiceVec.size(); };
};
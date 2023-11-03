#pragma once

//// Lambert Material type
//struct LambertInfo
//{
//	float diffuse[3];
//	float ambient[3];
//	float emissive[3];
//	float bump[3];
//	float alpha;
//};
//
// Phong Material Info
struct PhongInfo
{
	float diffuse[3];
	float ambient[3];
	float emissive[3];
	float bump[3];
	float specular[3];
	float reflection[3];
	float transparency; // blender����o�͂���ƁA1 - �ݒ�l���ǂݍ��܂��
};

struct FBXVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	unsigned int bone_index1[3];
	unsigned int bone_index2[3];
	float bone_weight1[3];
	float bone_weight2[3];
	float tangent[3];
	float biNormal[3];
	float vNormal[3];
};

//struct FBXVertex
//{
//	float pos[3];
//	float normal[3];
//	float uv[2];
//	unsigned short bone_index[6];
//	float bone_weight[6];
//};

struct VertexInfo
{
	std::vector<FBXVertex> vertices;
	std::vector<unsigned int> indices;
};

class FBXInfoManager
{
private:

	std::string modelPath;

	//FbxManager* manager = nullptr;
	FbxScene* scene = nullptr;

	void ReadFBXFile();

	std::unordered_map<std::string, VertexInfo> materialNameAndVertexInfo;
	std::vector<std::pair<std::string, VertexInfo>> finalVertexDrawOrder; // material name and VectorInfo(vertex number, index number) pair
	std::vector<std::pair<std::string, PhongInfo>> finalPhongMaterialOrder; // material name and PhongInfo(diffuse, ambient, emssive, bump, specular, reflection) pair
	std::vector<std::pair<std::string, std::string>> materialAndTexturenameInfo; // material name and used texture file path pair

	std::map<int, std::vector<int>> addtionalVertexIndexByApplication; // CreateNewVertexIndex()�ɂ���Ēǉ����ꂽ�V�K�C���f�b�N�X�BCluster�ɂ͏�񂪂Ȃ����߁A��XindexWithBonesNumAndWeight���ɂ���Ɋ�Â��C���f�b�N�X�ƃ{�[���E�E�F�C�g���R�s�[�A�ǉ�����
	// indexWithBonesNumAndWeight��map int ��AnimationNameAndBoneNameWithTranslationMatrix�̎n�߂�map int��cluster_index���Ȃ킿�{�[���C���f�b�N�X�Ɋ�Â��Ă���B
	// indexWithBonesNumAndWeight�͑S���_�C���f�b�N�X�ƁA����炪�e�����󂯂�{�[���C���f�b�N�X�Ɗe�E�F�C�g���A
	// AnimationNameAndBoneNameWithTranslationMatrix�̓x�C�N���ꂽ�e�A�j���[�V���������ɁA�{�[���C���f�b�N�X���ɃA�j���[�V�����̃t���[�����Ɖ�]�E���s�ړ��s��(XMMATRIX)��map�̓���q�ɂ��Ċi�[���Ă���B
	// �����̓Z�b�g�ŗ��p����B��҂���A�j���[�V���������t���[�����ɔ����o���Ăǂ̃{�[�������̃t���[���ŏ����p���t�s��ɑ΂��Ăǂ̉�]�E���s�ړ��s����|���邩���ʂ��邪�A����ɂ���ĉe�����󂯂钸�_�͑O�҂ƃ{�[���C���f�b�N�X��ʂ��Ĕ��ʂ��āA
	// �{�[���E�F�C�g�ƂƂ��ɉ�]�E���s�ړ��s���Vertex Shader�Ɉ����n���悤�ɏ�������B
	std::map<int, std::map<int, float>> indexWithBonesNumAndWeight;
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> animationNameAndBoneNameWithTranslationMatrix;
	std::map<int, XMMATRIX> bonesInitialPostureMatrix; // �e�{�[���������p���ɖ߂����߂�

	bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);

	int meshVertIndexStart = 0;
	int lastMeshIndexNumByCluster = 0; // �O�񃁃b�V���̃N���X�^�[����ǂݎ�����C���f�b�N�X���B���̃��b�V���̃C���f�b�N�X�ɑ����āA�ʂ��ԍ��ɂ���B

	std::vector<const char*> textureType = { "DiffuseColor", "NormalMap", "SpecularFactor", "ReflectionFactor", "TransparencyFactor"};

	std::vector<std::vector<float>> vertexInfoList;
	//std::vector<FBXVertex> vertices;

	void ProcessTangent(FbxMesh* mesh, std::string materialName, int nameCnt, int index);
	std::map<std::string, std::vector<unsigned int>> indexOFTangentBinormalNormalByMaterialName;

	std::map<std::string, std::map<int, std::vector<float>>> indexWithTangentBinormalNormalByMaterialName;
	int testCnt = 0;
	bool isBonesInitialPostureMatrixFilled = false;

	//FbxDouble3 localTransition; // ���f���̃��[�J��X,Y,Z���W(blender�œK�p�����ꍇ�͎擾�s�\)
	//FbxDouble3 localRotation; // ���f���̃��[�J��X,Y,Z��]�p�x(blender�Ŋp�x�K�p�����ꍇ�͎擾�s�\)
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> localPosAndRotOfMesh; // ���b�V�����̃��[�J�����W�E��]
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> localPosAndRotOfOBB; // OBB���̃��[�J�����W�E��]

	std::vector<std::pair<std::string, VertexInfo>> vertexListOfOBB;

	std::string testTargetPath = /*"C:\\Users\\RyoTaka\\Documents\\RenderingDemoRebuild\\FBX\\NewConnan_ZDir.fbx"*/"C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_Test.fbx";

public:
	//Get Singleton Instance
	static FBXInfoManager& Instance();
	int Init(std::string _modelPath);
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfo() { return finalVertexDrawOrder; };
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfoOfOBB() { return vertexListOfOBB; };
	std::vector<std::pair<std::string, PhongInfo>> GetPhongMaterialParamertInfo() { return finalPhongMaterialOrder; };
	std::vector<std::pair<std::string, std::string>> GetMaterialAndTexturePath() { return materialAndTexturenameInfo; };
	//std::map<int, std::map<int, float>> GetIndexWithBonesNumAndWeight() { return indexWithBonesNumAndWeight; };
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> GetAnimationNameAndBoneNameWithTranslationMatrix() { return animationNameAndBoneNameWithTranslationMatrix; };
	std::map<int, XMMATRIX> GetBonesInitialPostureMatrix() { return bonesInitialPostureMatrix; };
	//FbxDouble3 GetLocalTransition() { return localTransition; };
	//FbxDouble3 GetLocalRotation() { return localRotation; };
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> GetLocalPosAndRotOfMesh() { return localPosAndRotOfMesh; };
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> GetLocalPosAndRotOfOBB() { return localPosAndRotOfOBB; };
};
#pragma once

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

struct VertexInfo
{
	std::vector<FBXVertex> vertices;
	std::vector<unsigned int> indices;
};

class FBXInfoManager
{
private:

	std::string modelPath;

	std::vector<std::pair<std::string, VertexInfo>> finalVertexDrawOrder; // material name and VectorInfo(vertex number, index number) pair
	std::vector<std::pair<std::string, PhongInfo>> finalPhongMaterialOrder; // material name and PhongInfo(diffuse, ambient, emssive, bump, specular, reflection) pair
	std::vector<std::pair<std::string, std::string>> materialAndTexturenameInfo; // material name and used texture file path pair
	std::map<int, std::map<int, float>> indexWithBonesNumAndWeight;
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> animationNameAndBoneNameWithTranslationMatrix;
	std::map<int, XMMATRIX> bonesInitialPostureMatrix; // �e�{�[���������p���ɖ߂����߂�

	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> localPosAndRotOfMesh; // ���b�V�����̃��[�J�����W�E��]
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> localPosAndRotOfOBB; // OBB���̃��[�J�����W�E��]

	std::vector<std::pair<std::string, VertexInfo>> vertexListOfOBB;

public:
	//Get Singleton Instance
	static FBXInfoManager& Instance();
	int Init(std::string _modelPath);
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfo() { return finalVertexDrawOrder; };
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfoOfOBB() { return vertexListOfOBB; };
	std::vector<std::pair<std::string, PhongInfo>> GetPhongMaterialParamertInfo() { return finalPhongMaterialOrder; };
	std::vector<std::pair<std::string, std::string>> GetMaterialAndTexturePath() { return materialAndTexturenameInfo; };

	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> GetAnimationNameAndBoneNameWithTranslationMatrix() { return animationNameAndBoneNameWithTranslationMatrix; };
	std::map<int, XMMATRIX> GetBonesInitialPostureMatrix() { return bonesInitialPostureMatrix; };

	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> GetLocalPosAndRotOfMesh() { return localPosAndRotOfMesh; };
	std::map<std::string, std::pair<XMFLOAT3, XMFLOAT3>> GetLocalPosAndRotOfOBB() { return localPosAndRotOfOBB; };
};
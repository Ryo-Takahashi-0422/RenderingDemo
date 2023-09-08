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
	float transparency; // blenderから出力すると、1 - 設定値が読み込まれる
};

struct FBXVertex
{
	float pos[3];
	float normal[3];
	float uv[2];
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

	void ReadFBXFile(FbxNode* node, const std::string& filePath);

	std::unordered_map<std::string, VertexInfo> materialNameAndVertexInfo;
	std::vector<std::pair<std::string, VertexInfo>> finalVertexDrawOrder; // material name and VectorInfo(vertex number, index number) pair
	std::vector<std::pair<std::string, PhongInfo>> finalPhongMaterialOrder; // material name and PhongInfo(diffuse, ambient, emssive, bump, specular, reflection) pair
	std::vector<std::pair<std::string, std::string>> materialAndTexturenameInfo; // material name and used texture file path pair

	std::map<int, std::vector<int>> addtionalVertexIndexByApplication; // CreateNewVertexIndex()によって追加された新規インデックス。Clusterには情報がないため、後々indexWithBonesNumAndWeight内にこれに基づきインデックスとボーン・ウェイトをコピー、追加する
	// indexWithBonesNumAndWeightのmap int とAnimationNameAndBoneNameWithTranslationMatrixの始めのmap intはcluster_indexすなわちボーンインデックスに基づいている。
	// indexWithBonesNumAndWeightは全頂点インデックスと、それらが影響を受けるボーンインデックスと各ウェイトを、
	// AnimationNameAndBoneNameWithTranslationMatrixはベイクされた各アニメーション名毎に、ボーンインデックス毎にアニメーションのフレーム数と回転・平行移動行列(XMMATRIX)をmapの入れ子にして格納している。
	// これらはセットで利用する。後者からアニメーション情報をフレーム毎に抜き出してどのボーンがそのフレームで初期姿勢逆行列に対してどの回転・平行移動行列を掛けるか判別するが、それによって影響を受ける頂点は前者とボーンインデックスを通じて判別して、
	// ボーンウェイトとともに回転・平行移動行列をVertex Shaderに引き渡すように処理する。
	std::map<int, std::map<int, float>> indexWithBonesNumAndWeight;
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> animationNameAndBoneNameWithTranslationMatrix;
	std::map<int, XMMATRIX> bonesInitialPostureMatrix; // 各ボーンを初期姿勢に戻すための

	bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);

	int meshVertIndexStart = 0;
	int lastMeshIndexNumByCluster = 0; // 前回メッシュのクラスターから読み取ったインデックス数。次のメッシュのインデックスに足して、通し番号にする。

	std::vector<const char*> textureType = { "DiffuseColor", "NormalMap", "SpecularFactor", "ReflectionFactor", "TransparencyFactor"};

	std::vector<std::vector<float>> vertexInfoList;
	//std::vector<FBXVertex> vertices;

	void ProcessTangent(FbxMesh* mesh, std::string materialName, int num, int index);
	void ProcessBinormal(FbxMesh* mesh, std::string materialName, int num, int index);
	void ProcessNormal(FbxMesh* mesh, std::string materialName, int num, int index);
	std::map<std::string, std::map<int, std::vector<float>>> indexWithTangentBinormalNormalByMaterialName;


public:
	//Get Singleton Instance
	static FBXInfoManager& Instance();
	int Init();
	std::vector<std::pair<std::string, VertexInfo>> GetIndiceAndVertexInfo() { return finalVertexDrawOrder; };
	std::vector<std::pair<std::string, PhongInfo>> GetPhongMaterialParamertInfo() { return finalPhongMaterialOrder; };
	std::vector<std::pair<std::string, std::string>> GetMaterialAndTexturePath() { return materialAndTexturenameInfo; };
	std::map<int, std::map<int, float>> GetIndexWithBonesNumAndWeight() { return indexWithBonesNumAndWeight; };
	std::map <std::string, std::map<int, std::map<int, XMMATRIX>>> GetAnimationNameAndBoneNameWithTranslationMatrix() { return animationNameAndBoneNameWithTranslationMatrix; };
	std::map<int, XMMATRIX> GetBonesInitialPostureMatrix() { return bonesInitialPostureMatrix; };
};
#include <stdafx.h>
#include <FBXInfoManager.h>

int FBXInfoManager::Init()
{
    // create manager
    manager = FbxManager::Create();

    // create IOSetting
    FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);

    // create Importer
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (importer->Initialize("C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking_Tri.fbx", -1, manager->GetIOSettings()) == false) {

        return -1; // failed
    }

    // Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // �V�[���𗬂����񂾂�Importer�͉������OK

    //FbxGeometryConverter converter(manager);
    //// �|���S�����O�p�`�ɂ���
    //converter.Triangulate(scene, true);

    // Scene���
    // ���[�g�m�[�h���擾
    FbxNode* root = scene->GetRootNode();
    if (root != 0) {
        // �Ԃ牺�����Ă���m�[�h�̖��O���
        enumNodeNamesAndAttributes(root, 0, "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking_Tri.fbx");
    }

    // �}�l�[�W�����
    // �֘A���邷�ׂẴI�u�W�F�N�g����������
    scene->Destroy();
    manager->Destroy();

    return 0;
}


void FBXInfoManager::printSpace(int count)
{
    for (int i = 0; i < count; ++i)
        printf(" ");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���������b�V���͓ǂݍ��߂邪�`��o���Ȃ��BFBX SDK�̎d�l...?�@��������������炸�A�ǂݍ��݃��f�������b�V���������邱�Ƃŉ������B
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FBXInfoManager::enumNodeNamesAndAttributes(FbxNode* node, int indent, const std::string& filePath)
{
    //printSpace(indent);
    const char* typeNames[] = {
        "eUnknown", "eNull", "eMarker", "eSkeleton", "eMesh", "eNurbs",
        "ePatch", "eCamera", "eCameraStereo", "eCameraSwitcher", "eLight",
        "eOpticalReference", "eOpticalMarker", "eNurbsCurve", "eTrimNurbsSurface",
        "eBoundary", "eNurbsSurface", "eShape", "eLODGroup", "eSubDiv",
        "eCachedEffect", "eLine"
    };
    const char* name = node->GetName();
    int attrCount = node->GetNodeAttributeCount();
    //if (attrCount == 0) {
    //    printf("%s\n", name);
    //}
    //else {
    //    printf("%s (", name);
    //}
    for (int i = 0; i < attrCount; ++i) {
        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
        FbxNodeAttribute::EType type = attr->GetAttributeType();
        //printf("%s", typeNames[type]);
        //if (i + 1 == attrCount) {
        //    printf(")\n");
        //}
        //else {
        //    printf(", ");
        //}

        //printf("%s\n", typeNames[type]);
        if (typeNames[type] == "eMesh")
        {
            // ���_x,y,z���W�̒��o�y��vertices�ւ̊i�[
            fbxMeshes.resize(meshCnt);
            fbxMesh = (FbxMesh*)attr;
            fbxMeshes.emplace_back(fbxMesh);
            ++meshCnt;

            // UV�Z�b�g���̎擾
            // * ���݂̎������Ƃ�1��UV�Z�b�g���ɂ����Ή����Ă��Ȃ�...
            FbxStringList uvSetNameList;
            fbxMesh->GetUVSetNames(uvSetNameList);
            const char* uvSetName = uvSetNameList.GetStringAt(0);
            // ���_���W���̃��X�g�𐶐�
            std::vector<std::vector<float>> vertexInfoList;
            for (int i = 0; i < fbxMesh->GetControlPointsCount(); i++)
            {
                // ���_���W��ǂݍ���Őݒ�
                auto point = fbxMesh->GetControlPointAt(i);
                std::vector<float> vertex;
                vertex.push_back(point[0]);
                vertex.push_back(point[1]);
                vertex.push_back(point[2]);
                vertexInfoList.push_back(vertex);
            }
            // ���_���̏����擾����
            std::vector<unsigned short> indices;
            std::vector<unsigned short> indicesFiexed4DirectX;
            std::vector<std::array<int, 2>> oldNewIndexPairList;
            for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // �|���S�����̃��[�v
            {
                for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // ���_���̃��[�v
                {
                    printf("%d\n", fbxMesh->GetPolygonCount());
                    printf("%d\n", fbxMesh->GetPolygonSize(polIndex));
                    // �C���f�b�N�X���W
                    auto vertexIndex = fbxMesh->GetPolygonVertex(polIndex, polVertexIndex);
                    // ���_���W
                    std::vector<float> vertexInfo = vertexInfoList[vertexIndex];
                    // �@�����W
                    FbxVector4 normalVec4;
                    fbxMesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);
                    // UV���W
                    FbxVector2 uvVec2;
                    bool isUnMapped;
                    fbxMesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);
                    // �C���f�b�N�X���W�̃`�F�b�N�ƍč̔�
                    if (!IsExistNormalUVInfo(vertexInfo))
                    {
                        // �@�����W��UV���W�����ݒ�̏ꍇ�A���_���ɕt�^���čĐݒ�
                        vertexInfoList[vertexIndex] = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
                    }
                    else if (!IsSetNormalUV(vertexInfo, normalVec4, uvVec2))
                    {
                        // �����꒸�_�C���f�b�N�X�̒��Ŗ@�����W��UV���W���قȂ�ꍇ�A
                        // �V���Ȓ��_�C���f�b�N�X�Ƃ��č쐬����
                        vertexIndex = CreateNewVertexIndex(vertexInfo, normalVec4, uvVec2, vertexInfoList, vertexIndex, oldNewIndexPairList);
                    }
                    // �C���f�b�N�X���W��ݒ�

                    indices.push_back(vertexIndex);
                }
            }
            // ���_���𐶐�
            std::vector<FBXVertex> vertices;
            for (int i = 0; i < vertexInfoList.size(); i++)
            {
                std::vector<float> vertexInfo = vertexInfoList[i];
                vertices.push_back(FBXVertex{
                    {
                        -vertexInfo[0], vertexInfo[1], vertexInfo[2] // �Ȃ���Y���~���[���ꂽ��Ԃ̒��_���W�ɂȂ��Ă���B�̂ŁA�Ƃ肠����X���W�l��-1���Ƃ��B
                    },
                    {
                        vertexInfo[3], vertexInfo[4], vertexInfo[5] // ���̉e���ɒ���
                    },
                    {
                        vertexInfo[6], /*1.0f - */vertexInfo[7] // Blender�ō쐬�����ꍇ�AV�l�͔��]������
                    }
                    });
            }

            //// ����n�C���f�N�X�ɏC��
            //for (int i = 0; i < /*fbxMesh->GetPolygonCount()*/indices.size() / 3; ++i)
            //{
            //    // 2 => 1 => 0�ɂ��Ă�͍̂���n�΍�
            //    indicesFiexed4DirectX.push_back(indices[i * 3 + 2]);
            //    indicesFiexed4DirectX.push_back(indices[i * 3 + 1]);
            //    indicesFiexed4DirectX.push_back(indices[i * 3]);
            //}

            finalInfo[name] = { vertices, indices/*indicesFiexed4DirectX*/ };
        }

    }

    int childCount = node->GetChildCount();

    for (int i = 0; i < childCount; ++i) {
        enumNodeNamesAndAttributes(node->GetChild(i), indent + 1, "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking_Tri.fbx");
    }
}

// �@���AUV��񂪑��݂��Ă��邩�H
bool FBXInfoManager::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8; // ���_3 + �@��3 + UV2
}
// ���_���𐶐�
std::vector<float> FBXInfoManager::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo;
    // �ʒu���W
    newVertexInfo.push_back(vertexInfo[0]);
    newVertexInfo.push_back(vertexInfo[1]);
    newVertexInfo.push_back(vertexInfo[2]);
    // �@�����W
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    // UV���W
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}

// �V���Ȓ��_�C���f�b�N�X�𐶐�����
int FBXInfoManager::CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
    std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList)
{
    // �쐬�ς̏ꍇ�A�Y���̃C���f�b�N�X��Ԃ�
    for (int i = 0; i < oldNewIndexPairList.size(); i++)
    {
        int newIndex = oldNewIndexPairList[i][1];
        if (oldIndex == oldNewIndexPairList[i][0]
            && IsSetNormalUV(vertexInfoList[newIndex], normalVec4, uvVec2))
        {
            return newIndex;
        }
    }
    // �쐬�ςłȂ��ꍇ�A�V���Ȓ��_�C���f�b�N�X�Ƃ��č쐬
    std::vector<float> newVertexInfo = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
    vertexInfoList.push_back(newVertexInfo);
    // �쐬�����C���f�b�N�X����ݒ�
    int newIndex = vertexInfoList.size() - 1;
    std::array<int, 2> oldNewIndexPair{ oldIndex , newIndex };
    oldNewIndexPairList.push_back(oldNewIndexPair);
    return newIndex;
}
// vertexInfo�ɖ@���AUV���W���ݒ�ς��ǂ����H
bool FBXInfoManager::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    // �@���AUV���W�����l�Ȃ�ݒ�ςƂ݂Ȃ�
    return fabs(vertexInfo[3] - normalVec4[0]) < FLT_EPSILON
        && fabs(vertexInfo[4] - normalVec4[1]) < FLT_EPSILON
        && fabs(vertexInfo[5] - normalVec4[2]) < FLT_EPSILON
        && fabs(vertexInfo[6] - uvVec2[0]) < FLT_EPSILON
        && fabs(vertexInfo[7] - uvVec2[1]) < FLT_EPSILON;
}
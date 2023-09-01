#include <stdafx.h>
#include <FBXInfoManager.h>

int FBXInfoManager::Init()
{
    modelPath = "C:\\Users\\RyoTaka\\Desktop\\batllefield\\twocharTest.fbx";
    //modelPath = "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking_Tri.fbx";
    //modelPath = "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\BattleTank.fbx";
    modelPath = "C:\\Users\\RyoTaka\\Desktop\\batllefield\\BattleField_fixed.fbx";
    
    
    // create manager
    manager = FbxManager::Create();

    // create IOSetting
    FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);

    // create Importer
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (importer->Initialize(modelPath.c_str(), -1, manager->GetIOSettings()) == false) {

        return -1; // failed
    }

    // Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // �V�[���𗬂����񂾂�Importer�͉������OK

    FbxGeometryConverter converter(manager);
    // �|���S�����O�p�`�ɂ���
    //converter.Triangulate(scene, true);

    // FbxGeometryConverter converter(manager);
    // �SMesh����
    converter.SplitMeshesPerMaterial(scene, true);

    // Scene���
    // ���[�g�m�[�h���擾
    FbxNode* root = scene->GetRootNode();
    if (root != 0) {
        // �Ԃ牺�����Ă���m�[�h�̖��O���
        enumNodeNamesAndAttributes(root, 0, modelPath);
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
            std::vector<unsigned int> indices;
            std::vector<unsigned int> indicesFiexed4DirectX;
            std::vector<std::array<int, 2>> oldNewIndexPairList;
            for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // �|���S�����̃��[�v
            {
                for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // ���_���̃��[�v
                {
                    //printf("%d\n", fbxMesh->GetPolygonCount());
                    //printf("%d\n", fbxMesh->GetPolygonSize(polIndex));

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

                    vertexIndex += meshVertIndexStart; // �������ꂽ���b�V���̃C���f�b�N�X�́A�������Ȃ��Ɣԍ���0����U�蒼�����B����A�C���f�b�N�X�̓��b�V������������Ă��悤���P��̂��̂��낤���ʂ��ԍ��Ȃ̂ŁA
                                                       // ���b�V���𕪊�����ꍇ�͈�O�ɓǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�u�ő�̒l + 1�v�������̂�ǉ�����K�v������B
                    indices.push_back(vertexIndex);
                    
                }
                
            }

            // �O��ǂݍ��񂾃��b�V���̃C���f�b�N�X�ԍ��̓��A�ő�l�𒊏o����
            auto iter = std::max_element(indices.begin(), indices.end());
            size_t index = std::distance(indices.begin(), iter);
            meshVertIndexStart = indices[index] + 1;                       

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

            char newName[32];
            
            // �}�e���A�����ɕ��������̂ɍ��킹�ă��l�[��
            sprintf(newName, "%s%d", name, nameCnt);
            name = newName;
            
            finalInfo[name] = { vertices, indices/*indicesFiexed4DirectX*/ };
            auto itFI = finalInfo.begin();
            for (int i = 0; i <= nameCnt; i++)
            {
                ++itFI;
            }
            
            // ���b�V����ǂݍ��񂾏��Ԃɐ��񂷂�B
            // �����������b�V���̒��_��`�悷��Ƃ��A�ʂ̃��b�V���̃C���f�b�N�X�����荞�ނƕ`�悪���s����B
            // �܂��A�X�e�[�W�Ȃǃ��b�V�����������݂���ꍇ�͓��ꂵ�Ă����B���ꂵ�Ȃ��Ƃ��ꂼ��̃��b�V���̃��[�J�����W�ŕ`�悳��邽�߁A���b�V�������b�v������Ԃł̕`��ɂȂ��Ă��܂��B
            // Align meshes in the order in which they are read.
            // When drawing the vertices of a divided mesh, if the index of another mesh interrupts the drawing, the drawing will fail.
            // Also, if there are multiple meshes, such as stages, they should be unified.Otherwise, the local coordinates of each mesh will be used for drawing, resulting in wrapped meshes.
            finalOrder.resize(nameCnt + 1);
            finalOrder.at(nameCnt).first = name;
            finalOrder.at(nameCnt).second.vertices = vertices;
            finalOrder.at(nameCnt).second.indices = indices;

            nameCnt += 1;
            //name = node->GetName();

            // �}�e���A�����擾
            FbxNode* node = fbxMesh->GetNode();
            if (node == 0) 
            {
                //return;
                continue;
            }

            // �}�e���A���̐�
            int materialNum = node->GetMaterialCount();
            if (materialNum == 0) 
            {
                //return;
                continue;
            }

            // �}�e���A�������擾
            //for (int i = 0; i < materialNum ; ++i) 
            //{
                FbxSurfaceMaterial* material = node->GetMaterial(i);
                
                if (material != 0) 
                {
                    std::string type;
                    type = material->GetClassId().GetName();

                    char newMaterialName[32];

                    //// �}�e���A�����ɕ��������̂ɍ��킹�ă��l�[��
                    //sprintf(newMaterialName, "%s%d", name, materialNameCnt);
                    //name = newMaterialName;
                    //++materialNameCnt;

                    // �}�e���A�����
                    // Lambert��Phong��

                    if (type == "FbxSurfaceLambert") {
                        
                        // Lambert�Ƀ_�E���L���X�g
                        FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

                        // Diffuse
                        m_LambertInfo[name][i].diffuse[0] = (float)lambert->Diffuse.Get().mData[0];
                        m_LambertInfo[name][i].diffuse[1] = (float)lambert->Diffuse.Get().mData[1];
                        m_LambertInfo[name][i].diffuse[2] = (float)lambert->Diffuse.Get().mData[2];

                        // Ambient
                        m_LambertInfo[name][i].ambient[0] = (float)lambert->Ambient.Get().mData[0];
                        m_LambertInfo[name][i].ambient[1] = (float)lambert->Ambient.Get().mData[1];
                        m_LambertInfo[name][i].ambient[2] = (float)lambert->Ambient.Get().mData[2];

                        // emissive
                        m_LambertInfo[name][i].emissive[0] = (float)lambert->Emissive.Get().mData[0];
                        m_LambertInfo[name][i].emissive[1] = (float)lambert->Emissive.Get().mData[1];
                        m_LambertInfo[name][i].emissive[2] = (float)lambert->Emissive.Get().mData[2];

                        // bump
                        m_LambertInfo[name][i].bump[0] = (float)lambert->Bump.Get().mData[0];
                        m_LambertInfo[name][i].bump[1] = (float)lambert->Bump.Get().mData[1];
                        m_LambertInfo[name][i].bump[2] = (float)lambert->Bump.Get().mData[2];
                    }
                    else if (type == "FbxSurfacePhong") {

                        // Phong�Ƀ_�E���L���X�g
                        FbxSurfacePhong* phong = (FbxSurfacePhong*)material;
                        m_PhongInfo[name].resize(materialNum);
                        printf("%s\n", material->GetName());
                        // Diffuse
                        m_PhongInfo[name][i].diffuse[0] = (float)phong->Diffuse.Get().mData[0];
                        m_PhongInfo[name][i].diffuse[1] = (float)phong->Diffuse.Get().mData[1];
                        m_PhongInfo[name][i].diffuse[2] = (float)phong->Diffuse.Get().mData[2];

                        // Ambient
                        m_PhongInfo[name][i].ambient[0] = (float)phong->Ambient.Get().mData[0];
                        m_PhongInfo[name][i].ambient[1] = (float)phong->Ambient.Get().mData[1];
                        m_PhongInfo[name][i].ambient[2] = (float)phong->Ambient.Get().mData[2];

                        // emissive
                        m_PhongInfo[name][i].emissive[0] = (float)phong->Emissive.Get().mData[0];
                        m_PhongInfo[name][i].emissive[1] = (float)phong->Emissive.Get().mData[1];
                        m_PhongInfo[name][i].emissive[2] = (float)phong->Emissive.Get().mData[2];

                        // bump
                        m_PhongInfo[name][i].bump[0] = (float)phong->Bump.Get().mData[0];
                        m_PhongInfo[name][i].bump[1] = (float)phong->Bump.Get().mData[1];
                        m_PhongInfo[name][i].bump[2] = (float)phong->Bump.Get().mData[2];

                        // specular
                        m_PhongInfo[name][i].specular[0] = (float)phong->Specular.Get().mData[0];
                        m_PhongInfo[name][i].specular[1] = (float)phong->Specular.Get().mData[1];
                        m_PhongInfo[name][i].specular[2] = (float)phong->Specular.Get().mData[2];

                        // reflectivity
                        m_PhongInfo[name][i].reflection[0] = (float)phong->Reflection.Get().mData[0];
                        m_PhongInfo[name][i].reflection[1] = (float)phong->Reflection.Get().mData[1];
                        m_PhongInfo[name][i].reflection[2] = (float)phong->Reflection.Get().mData[2];

                        // shiness
                        m_PhongInfo[name][i].shineness = (float)phong->Shininess.Get();
                    }
                }
                name = node->GetName();
            //}
        }

    }



    int childCount = node->GetChildCount();

    for (int i = 0; i < childCount; ++i) {
        enumNodeNamesAndAttributes(node->GetChild(i), indent + 1, modelPath);
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
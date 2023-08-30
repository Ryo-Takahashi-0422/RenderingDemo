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
    if (importer->Initialize("C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx", -1, manager->GetIOSettings()) == false) {

        return -1; // failed
    }

    // Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // �V�[���𗬂����񂾂�Importer�͉������OK

    FbxGeometryConverter converter(manager);
    // �|���S�����O�p�`�ɂ���
    converter.Triangulate(scene, true);

    // Scene���
    // ���[�g�m�[�h���擾
    FbxNode* root = scene->GetRootNode();
    if (root != 0) {
        // �Ԃ牺�����Ă���m�[�h�̖��O���
        enumNodeNamesAndAttributes(root, 0, "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx", &fbxVertexInfo);
    }

    // �}�l�[�W�����
    // �֘A���邷�ׂẴI�u�W�F�N�g����������
    manager->Destroy();

    return 0;
}


void FBXInfoManager::printSpace(int count)
{
    for (int i = 0; i < count; ++i)
        printf(" ");
}

void FBXInfoManager::enumNodeNamesAndAttributes(FbxNode* node, int indent, const std::string& filePath, VertexInfo* vertexInfo)
{
    printSpace(indent);
    const char* typeNames[] = {
        "eUnknown", "eNull", "eMarker", "eSkeleton", "eMesh", "eNurbs",
        "ePatch", "eCamera", "eCameraStereo", "eCameraSwitcher", "eLight",
        "eOpticalReference", "eOpticalMarker", "eNurbsCurve", "eTrimNurbsSurface",
        "eBoundary", "eNurbsSurface", "eShape", "eLODGroup", "eSubDiv",
        "eCachedEffect", "eLine"
    };
    const char* name = node->GetName();
    int attrCount = node->GetNodeAttributeCount();
    if (attrCount == 0) {
        printf("%s\n", name);
    }
    else {
        printf("%s (", name);
    }
    for (int i = 0; i < attrCount; ++i) {
        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
        FbxNodeAttribute::EType type = attr->GetAttributeType();
        printf("%s", typeNames[type]);
        if (i + 1 == attrCount) {
            printf(")\n");
        }
        else {
            printf(", ");
        }

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
                        vertexInfo[0], vertexInfo[1], vertexInfo[2]
                    },
                    {
                        vertexInfo[3], vertexInfo[4], vertexInfo[5]
                    },
                    {
                        vertexInfo[6], 1.0f - vertexInfo[7] // Blender�ō쐬�����ꍇ�AV�l�͔��]������
                    }
                    });
            }

            *vertexInfo = 
            {
                vertices,
                indices
            };

            finalInfo[name] = { vertices, indices };
            

            //// ���_�o�b�t�@�̎擾
            //FbxVector4* vertices = fbxMesh->GetControlPoints();
            //// �C���f�b�N�X�o�b�t�@�̎擾
            //int* indices = fbxMesh->GetPolygonVertices();

            //// ���_���W�̐��̎擾
            //int polygon_vertex_count = fbxMesh->GetPolygonVertexCount();
            //vertNum += fbxMesh->GetControlPointsCount();

            //// GetPolygonVertexCount => ���_��
            //for (int i = 0; i < polygon_vertex_count; i++)
            //{
            //    VertexInfo vertexInfo = {};
            //    // �C���f�b�N�X�o�b�t�@���璸�_�ԍ����擾
            //    int index = indices[i];
            //    indiceVec.emplace_back(index);

            //    // ���_���W���X�g������W���擾����
            //    vertexInfo.pos[0] = vertices[index][0];
            //    vertexInfo.pos[1] = vertices[index][1];
            //    vertexInfo.pos[2] = vertices[index][2];

            //    if (indexAndVertexPosByMeshName[name].find(index) == indexAndVertexPosByMeshName[name].end())
            //    {
            //        indexAndVertexPosByMeshName[name][index][0] = vertices[index][0];
            //        indexAndVertexPosByMeshName[name][index][1] = vertices[index][1];
            //        indexAndVertexPosByMeshName[name][index][2] = vertices[index][2];
            //    }

            //    // �ǉ�
            //    m_VertexInfo[name].push_back(vertexInfo);
            //}

            //// ����n�C���f�N�X�ɏC��
            //for (int i = indiceCnt; i < /*fbxMesh->GetPolygonCount()*/indiceVec.size() / 3; ++i)
            //{
            //    // 2 => 1 => 0�ɂ��Ă�͍̂���n�΍�
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 2]);
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 1]);
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3]);
            //    ++indiceCnt;
            //}
            //           
            //// �@�����X�g�̎擾
            //FbxArray<FbxVector4> normals;
            //fbxMesh->GetPolygonVertexNormals(normals);
            //// �@���ݒ�
            //for (int i = 0; i < normals.Size(); i++)
            //{
            //    m_VertexInfo[name][i].normal_vec[0] = normals[i][0];
            //    m_VertexInfo[name][i].normal_vec[1] = normals[i][1];
            //    m_VertexInfo[name][i].normal_vec[2] = normals[i][2];
            //}

            //// uv�擾
            //int layerCount = fbxMesh->GetLayerCount();   // mesh��KFbxMesh
            //for (int i = 0; i < layerCount; ++i) 
            //{
            //    FbxLayer* layer = fbxMesh->GetLayer(i);
            //    FbxLayerElementUV* elem = layer->GetUVs();
            //    if (elem == 0) 
            //    {
            //        continue;
            //    }

            //    // UV�����擾
            //    // UV�̐��E�C���f�b�N�X
            //    int UVNum = elem->GetDirectArray().GetCount();
            //    int indexNum = elem->GetIndexArray().GetCount();
            //    int size = UVNum > indexNum ? UVNum : indexNum;
            //    XMFLOAT2* buffer = new XMFLOAT2[size];

            //    // �}�b�s���O���[�h�E���t�@�����X���[�h�ʂ�UV�擾
            //    FbxLayerElement::EMappingMode mappingMode = elem->GetMappingMode();
            //    FbxLayerElement::EReferenceMode refMode = elem->GetReferenceMode();

            //    if (mappingMode == FbxLayerElement::eByPolygonVertex)
            //    {
            //        if (refMode == FbxLayerElement::eDirect) 
            //        {
            //            // ���ڎ擾
            //            for (int i = 0; i < size; ++i) 
            //            {
            //                m_VertexInfo[name][i].uv[0] = (float)elem->GetDirectArray().GetAt(i)[0];
            //                m_VertexInfo[name][i].uv[1] = (float)elem->GetDirectArray().GetAt(i)[1];

            //                //m_VertexInfo[name][i].uvname = elem->GetName();
            //            }
            //        }
            //        else
            //            if (refMode == FbxLayerElement::eIndexToDirect) 
            //            {
            //                // �C���f�b�N�X����擾
            //                for (int i = 0; i < size; ++i) 
            //                {
            //                    int index = elem->GetIndexArray().GetAt(i);
            //                    indiceVecOfVec.emplace_back(index);
            //                    m_VertexInfo[name][i].uv[0] = (float)elem->GetDirectArray().GetAt(index)[0];
            //                    m_VertexInfo[name][i].uv[1] = (float)elem->GetDirectArray().GetAt(index)[1];

            //                    //m_VertexInfo[name][i].uvname = elem->GetName();
            //                    if (indexAndUVByMeshName[name].find(index) == indexAndUVByMeshName[name].end())
            //                    {
            //                        indexAndUVByMeshName[name][index][0] = (float)elem->GetDirectArray().GetAt(index)[0];
            //                        indexAndUVByMeshName[name][index][1] = (float)elem->GetDirectArray().GetAt(index)[1];
            //                    }
            //                }
            //            }
            //    }
            //}

            //// �}�e���A�����擾
            //FbxNode* node = fbxMesh->GetNode();
            //if (node == 0) 
            //{
            //    return;
            //}

            //// �}�e���A���̐�
            //int materialNum = node->GetMaterialCount();
            //if (materialNum == 0) 
            //{
            //    return;
            //}

            //// �}�e���A�������擾
            //for (int i = 0; i < materialNum ; ++i) 
            //{
            //    FbxSurfaceMaterial* material = node->GetMaterial(i);
            //    if (material != 0) 
            //    {
            //        std::string type;
            //        type = material->GetClassId().GetName();

            //        // �}�e���A�����
            //        // Lambert��Phong��

            //        if (type == "FbxSurfaceLambert") {
            //            
            //            // Lambert�Ƀ_�E���L���X�g
            //            FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

            //            // Diffuse
            //            m_VertexInfo[name][i].lamberInfo.diffuse[0] = (float)lambert->Diffuse.Get().mData[0];
            //            m_VertexInfo[name][i].lamberInfo.diffuse[1] = (float)lambert->Diffuse.Get().mData[1];
            //            m_VertexInfo[name][i].lamberInfo.diffuse[2] = (float)lambert->Diffuse.Get().mData[2];

            //            // Ambient
            //            m_VertexInfo[name][i].lamberInfo.ambient[0] = (float)lambert->Ambient.Get().mData[0];
            //            m_VertexInfo[name][i].lamberInfo.ambient[1] = (float)lambert->Ambient.Get().mData[1];
            //            m_VertexInfo[name][i].lamberInfo.ambient[2] = (float)lambert->Ambient.Get().mData[2];

            //            // emissive
            //            m_VertexInfo[name][i].lamberInfo.emissive[0] = (float)lambert->Emissive.Get().mData[0];
            //            m_VertexInfo[name][i].lamberInfo.emissive[1] = (float)lambert->Emissive.Get().mData[1];
            //            m_VertexInfo[name][i].lamberInfo.emissive[2] = (float)lambert->Emissive.Get().mData[2];

            //            // bump
            //            m_VertexInfo[name][i].lamberInfo.bump[0] = (float)lambert->Bump.Get().mData[0];
            //            m_VertexInfo[name][i].lamberInfo.bump[1] = (float)lambert->Bump.Get().mData[1];
            //            m_VertexInfo[name][i].lamberInfo.bump[2] = (float)lambert->Bump.Get().mData[2];
            //        }
            //        else if (type == "FbxSurfacePhong") {

            //            // Phong�Ƀ_�E���L���X�g
            //            FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

            //            // Diffuse
            //            m_VertexInfo[name][i].phongInfo.diffuse[0] = (float)phong->Diffuse.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.diffuse[1] = (float)phong->Diffuse.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.diffuse[2] = (float)phong->Diffuse.Get().mData[2];

            //            // Ambient
            //            m_VertexInfo[name][i].phongInfo.ambient[0] = (float)phong->Ambient.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.ambient[1] = (float)phong->Ambient.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.ambient[2] = (float)phong->Ambient.Get().mData[2];

            //            // emissive
            //            m_VertexInfo[name][i].phongInfo.emissive[0] = (float)phong->Emissive.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.emissive[1] = (float)phong->Emissive.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.emissive[2] = (float)phong->Emissive.Get().mData[2];

            //            // bump
            //            m_VertexInfo[name][i].phongInfo.bump[0] = (float)phong->Bump.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.bump[1] = (float)phong->Bump.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.bump[2] = (float)phong->Bump.Get().mData[2];

            //            // specular
            //            m_VertexInfo[name][i].phongInfo.specular[0] = (float)phong->Specular.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.specular[1] = (float)phong->Specular.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.specular[2] = (float)phong->Specular.Get().mData[2];

            //            // reflectivity
            //            m_VertexInfo[name][i].phongInfo.reflection[0] = (float)phong->Reflection.Get().mData[0];
            //            m_VertexInfo[name][i].phongInfo.reflection[1] = (float)phong->Reflection.Get().mData[1];
            //            m_VertexInfo[name][i].phongInfo.reflection[2] = (float)phong->Reflection.Get().mData[2];

            //            // shiness
            //            m_VertexInfo[name][i].phongInfo.shineness = (float)phong->Shininess.Get();
            //        }
            //    }
            //}
        }

    }

    int childCount = node->GetChildCount();
    for (int i = 0; i < childCount; ++i) {
        enumNodeNamesAndAttributes(node->GetChild(i), indent + 1, "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx", &fbxVertexInfo);
    }

    // �}�l�[�W���[�A�V�[���̔j��
    //scene->Destroy();
    //manager->Destroy();
    // �ԋp�l�ɐݒ�


    fbxModelInfo["C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx"] = fbxPolygonMap;
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
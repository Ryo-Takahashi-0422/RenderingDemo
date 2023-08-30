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

    // SceneオブジェクトにFBXファイル内の情報を流し込む
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // シーンを流し込んだらImporterは解放してOK

    FbxGeometryConverter converter(manager);
    // ポリゴンを三角形にする
    converter.Triangulate(scene, true);

    // Scene解析
    // ルートノードを取得
    FbxNode* root = scene->GetRootNode();
    if (root != 0) {
        // ぶら下がっているノードの名前を列挙
        enumNodeNamesAndAttributes(root, 0, "C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx", &fbxVertexInfo);
    }

    // マネージャ解放
    // 関連するすべてのオブジェクトが解放される
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
            // 頂点x,y,z座標の抽出及びverticesへの格納
            fbxMeshes.resize(meshCnt);
            fbxMesh = (FbxMesh*)attr;
            fbxMeshes.emplace_back(fbxMesh);
            ++meshCnt;

            // UVセット名の取得
            // * 現在の実装だとは1つのUVセット名にしか対応していない...
            FbxStringList uvSetNameList;
            fbxMesh->GetUVSetNames(uvSetNameList);
            const char* uvSetName = uvSetNameList.GetStringAt(0);
            // 頂点座標情報のリストを生成
            std::vector<std::vector<float>> vertexInfoList;
            for (int i = 0; i < fbxMesh->GetControlPointsCount(); i++)
            {
                // 頂点座標を読み込んで設定
                auto point = fbxMesh->GetControlPointAt(i);
                std::vector<float> vertex;
                vertex.push_back(point[0]);
                vertex.push_back(point[1]);
                vertex.push_back(point[2]);
                vertexInfoList.push_back(vertex);
            }
            // 頂点毎の情報を取得する
            std::vector<unsigned short> indices;
            std::vector<std::array<int, 2>> oldNewIndexPairList;
            for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // ポリゴン毎のループ
            {
                for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // 頂点毎のループ
                {
                    printf("%d\n", fbxMesh->GetPolygonCount());
                    printf("%d\n", fbxMesh->GetPolygonSize(polIndex));
                    // インデックス座標
                    auto vertexIndex = fbxMesh->GetPolygonVertex(polIndex, polVertexIndex);
                    // 頂点座標
                    std::vector<float> vertexInfo = vertexInfoList[vertexIndex];
                    // 法線座標
                    FbxVector4 normalVec4;
                    fbxMesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);
                    // UV座標
                    FbxVector2 uvVec2;
                    bool isUnMapped;
                    fbxMesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);
                    // インデックス座標のチェックと再採番
                    if (!IsExistNormalUVInfo(vertexInfo))
                    {
                        // 法線座標とUV座標が未設定の場合、頂点情報に付与して再設定
                        vertexInfoList[vertexIndex] = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
                    }
                    else if (!IsSetNormalUV(vertexInfo, normalVec4, uvVec2))
                    {
                        // ＊同一頂点インデックスの中で法線座標かUV座標が異なる場合、
                        // 新たな頂点インデックスとして作成する
                        vertexIndex = CreateNewVertexIndex(vertexInfo, normalVec4, uvVec2, vertexInfoList, vertexIndex, oldNewIndexPairList);
                    }
                    // インデックス座標を設定
                    indices.push_back(vertexIndex);
                }
            }
            // 頂点情報を生成
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
                        vertexInfo[6], 1.0f - vertexInfo[7] // Blenderで作成した場合、V値は反転させる
                    }
                    });
            }

            *vertexInfo = 
            {
                vertices,
                indices
            };

            finalInfo[name] = { vertices, indices };
            

            //// 頂点バッファの取得
            //FbxVector4* vertices = fbxMesh->GetControlPoints();
            //// インデックスバッファの取得
            //int* indices = fbxMesh->GetPolygonVertices();

            //// 頂点座標の数の取得
            //int polygon_vertex_count = fbxMesh->GetPolygonVertexCount();
            //vertNum += fbxMesh->GetControlPointsCount();

            //// GetPolygonVertexCount => 頂点数
            //for (int i = 0; i < polygon_vertex_count; i++)
            //{
            //    VertexInfo vertexInfo = {};
            //    // インデックスバッファから頂点番号を取得
            //    int index = indices[i];
            //    indiceVec.emplace_back(index);

            //    // 頂点座標リストから座標を取得する
            //    vertexInfo.pos[0] = vertices[index][0];
            //    vertexInfo.pos[1] = vertices[index][1];
            //    vertexInfo.pos[2] = vertices[index][2];

            //    if (indexAndVertexPosByMeshName[name].find(index) == indexAndVertexPosByMeshName[name].end())
            //    {
            //        indexAndVertexPosByMeshName[name][index][0] = vertices[index][0];
            //        indexAndVertexPosByMeshName[name][index][1] = vertices[index][1];
            //        indexAndVertexPosByMeshName[name][index][2] = vertices[index][2];
            //    }

            //    // 追加
            //    m_VertexInfo[name].push_back(vertexInfo);
            //}

            //// 左手系インデクスに修正
            //for (int i = indiceCnt; i < /*fbxMesh->GetPolygonCount()*/indiceVec.size() / 3; ++i)
            //{
            //    // 2 => 1 => 0にしてるのは左手系対策
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 2]);
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 1]);
            //    fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3]);
            //    ++indiceCnt;
            //}
            //           
            //// 法線リストの取得
            //FbxArray<FbxVector4> normals;
            //fbxMesh->GetPolygonVertexNormals(normals);
            //// 法線設定
            //for (int i = 0; i < normals.Size(); i++)
            //{
            //    m_VertexInfo[name][i].normal_vec[0] = normals[i][0];
            //    m_VertexInfo[name][i].normal_vec[1] = normals[i][1];
            //    m_VertexInfo[name][i].normal_vec[2] = normals[i][2];
            //}

            //// uv取得
            //int layerCount = fbxMesh->GetLayerCount();   // meshはKFbxMesh
            //for (int i = 0; i < layerCount; ++i) 
            //{
            //    FbxLayer* layer = fbxMesh->GetLayer(i);
            //    FbxLayerElementUV* elem = layer->GetUVs();
            //    if (elem == 0) 
            //    {
            //        continue;
            //    }

            //    // UV情報を取得
            //    // UVの数・インデックス
            //    int UVNum = elem->GetDirectArray().GetCount();
            //    int indexNum = elem->GetIndexArray().GetCount();
            //    int size = UVNum > indexNum ? UVNum : indexNum;
            //    XMFLOAT2* buffer = new XMFLOAT2[size];

            //    // マッピングモード・リファレンスモード別にUV取得
            //    FbxLayerElement::EMappingMode mappingMode = elem->GetMappingMode();
            //    FbxLayerElement::EReferenceMode refMode = elem->GetReferenceMode();

            //    if (mappingMode == FbxLayerElement::eByPolygonVertex)
            //    {
            //        if (refMode == FbxLayerElement::eDirect) 
            //        {
            //            // 直接取得
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
            //                // インデックスから取得
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

            //// マテリアル情報取得
            //FbxNode* node = fbxMesh->GetNode();
            //if (node == 0) 
            //{
            //    return;
            //}

            //// マテリアルの数
            //int materialNum = node->GetMaterialCount();
            //if (materialNum == 0) 
            //{
            //    return;
            //}

            //// マテリアル情報を取得
            //for (int i = 0; i < materialNum ; ++i) 
            //{
            //    FbxSurfaceMaterial* material = node->GetMaterial(i);
            //    if (material != 0) 
            //    {
            //        std::string type;
            //        type = material->GetClassId().GetName();

            //        // マテリアル解析
            //        // LambertかPhongか

            //        if (type == "FbxSurfaceLambert") {
            //            
            //            // Lambertにダウンキャスト
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

            //            // Phongにダウンキャスト
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

    // マネージャー、シーンの破棄
    //scene->Destroy();
    //manager->Destroy();
    // 返却値に設定


    fbxModelInfo["C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx"] = fbxPolygonMap;
}

// 法線、UV情報が存在しているか？
bool FBXInfoManager::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8; // 頂点3 + 法線3 + UV2
}
// 頂点情報を生成
std::vector<float> FBXInfoManager::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo;
    // 位置座標
    newVertexInfo.push_back(vertexInfo[0]);
    newVertexInfo.push_back(vertexInfo[1]);
    newVertexInfo.push_back(vertexInfo[2]);
    // 法線座標
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    // UV座標
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}

// 新たな頂点インデックスを生成する
int FBXInfoManager::CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
    std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList)
{
    // 作成済の場合、該当のインデックスを返す
    for (int i = 0; i < oldNewIndexPairList.size(); i++)
    {
        int newIndex = oldNewIndexPairList[i][1];
        if (oldIndex == oldNewIndexPairList[i][0]
            && IsSetNormalUV(vertexInfoList[newIndex], normalVec4, uvVec2))
        {
            return newIndex;
        }
    }
    // 作成済でない場合、新たな頂点インデックスとして作成
    std::vector<float> newVertexInfo = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
    vertexInfoList.push_back(newVertexInfo);
    // 作成したインデックス情報を設定
    int newIndex = vertexInfoList.size() - 1;
    std::array<int, 2> oldNewIndexPair{ oldIndex , newIndex };
    oldNewIndexPairList.push_back(oldNewIndexPair);
    return newIndex;
}
// vertexInfoに法線、UV座標が設定済かどうか？
bool FBXInfoManager::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    // 法線、UV座標が同値なら設定済とみなす
    return fabs(vertexInfo[3] - normalVec4[0]) < FLT_EPSILON
        && fabs(vertexInfo[4] - normalVec4[1]) < FLT_EPSILON
        && fabs(vertexInfo[5] - normalVec4[2]) < FLT_EPSILON
        && fabs(vertexInfo[6] - uvVec2[0]) < FLT_EPSILON
        && fabs(vertexInfo[7] - uvVec2[1]) < FLT_EPSILON;
}
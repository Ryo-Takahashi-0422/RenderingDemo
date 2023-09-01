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

    // SceneオブジェクトにFBXファイル内の情報を流し込む
    scene = FbxScene::Create(manager, "scene");
    importer->Import(scene);
    importer->Destroy(); // シーンを流し込んだらImporterは解放してOK

    FbxGeometryConverter converter(manager);
    // ポリゴンを三角形にする
    //converter.Triangulate(scene, true);

    // FbxGeometryConverter converter(manager);
    // 全Mesh分割
    converter.SplitMeshesPerMaterial(scene, true);

    // Scene解析
    // ルートノードを取得
    FbxNode* root = scene->GetRootNode();
    if (root != 0) {
        // ぶら下がっているノードの名前を列挙
        enumNodeNamesAndAttributes(root, 0, modelPath);
    }

    // マネージャ解放
    // 関連するすべてのオブジェクトが解放される
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
// ★複数メッシュは読み込めるが描画出来ない。FBX SDKの仕様...?　現状解決策分からず、読み込みモデルをメッシュ結合することで回避する。
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
            std::vector<unsigned int> indices;
            std::vector<unsigned int> indicesFiexed4DirectX;
            std::vector<std::array<int, 2>> oldNewIndexPairList;
            for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // ポリゴン毎のループ
            {
                for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // 頂点毎のループ
                {
                    //printf("%d\n", fbxMesh->GetPolygonCount());
                    //printf("%d\n", fbxMesh->GetPolygonSize(polIndex));

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

                    vertexIndex += meshVertIndexStart; // 分割されたメッシュのインデックスは、何もしないと番号が0から振り直される。一方、インデックスはメッシュが分割されていようが単一のものだろうが通し番号なので、
                                                       // メッシュを分割する場合は一つ前に読み込んだメッシュのインデックス番号の内、「最大の値 + 1」したものを追加する必要がある。
                    indices.push_back(vertexIndex);
                    
                }
                
            }

            // 前回読み込んだメッシュのインデックス番号の内、最大値を抽出する
            auto iter = std::max_element(indices.begin(), indices.end());
            size_t index = std::distance(indices.begin(), iter);
            meshVertIndexStart = indices[index] + 1;                       

            // 頂点情報を生成
            std::vector<FBXVertex> vertices;
            for (int i = 0; i < vertexInfoList.size(); i++)
            {
                std::vector<float> vertexInfo = vertexInfoList[i];
                vertices.push_back(FBXVertex{
                    {
                        -vertexInfo[0], vertexInfo[1], vertexInfo[2] // なぜかY軸ミラーされた状態の頂点座標になっている。ので、とりあえずX座標値に-1しとく。
                    },
                    {
                        vertexInfo[3], vertexInfo[4], vertexInfo[5] // ↑の影響に注意
                    },
                    {
                        vertexInfo[6], /*1.0f - */vertexInfo[7] // Blenderで作成した場合、V値は反転させる
                    }
                    });
            }

            //// 左手系インデクスに修正
            //for (int i = 0; i < /*fbxMesh->GetPolygonCount()*/indices.size() / 3; ++i)
            //{
            //    // 2 => 1 => 0にしてるのは左手系対策
            //    indicesFiexed4DirectX.push_back(indices[i * 3 + 2]);
            //    indicesFiexed4DirectX.push_back(indices[i * 3 + 1]);
            //    indicesFiexed4DirectX.push_back(indices[i * 3]);
            //}

            char newName[32];
            
            // マテリアル毎に分割されるのに合わせてリネーム
            sprintf(newName, "%s%d", name, nameCnt);
            name = newName;
            
            finalInfo[name] = { vertices, indices/*indicesFiexed4DirectX*/ };
            auto itFI = finalInfo.begin();
            for (int i = 0; i <= nameCnt; i++)
            {
                ++itFI;
            }
            
            // メッシュを読み込んだ順番に整列する。
            // 分割したメッシュの頂点を描画するとき、別のメッシュのインデックスが割り込むと描画が失敗する。
            // また、ステージなどメッシュが複数存在する場合は統一しておく。統一しないとそれぞれのメッシュのローカル座標で描画されるため、メッシュがラップした状態での描画になってしまう。
            // Align meshes in the order in which they are read.
            // When drawing the vertices of a divided mesh, if the index of another mesh interrupts the drawing, the drawing will fail.
            // Also, if there are multiple meshes, such as stages, they should be unified.Otherwise, the local coordinates of each mesh will be used for drawing, resulting in wrapped meshes.
            finalOrder.resize(nameCnt + 1);
            finalOrder.at(nameCnt).first = name;
            finalOrder.at(nameCnt).second.vertices = vertices;
            finalOrder.at(nameCnt).second.indices = indices;

            nameCnt += 1;
            //name = node->GetName();

            // マテリアル情報取得
            FbxNode* node = fbxMesh->GetNode();
            if (node == 0) 
            {
                //return;
                continue;
            }

            // マテリアルの数
            int materialNum = node->GetMaterialCount();
            if (materialNum == 0) 
            {
                //return;
                continue;
            }

            // マテリアル情報を取得
            //for (int i = 0; i < materialNum ; ++i) 
            //{
                FbxSurfaceMaterial* material = node->GetMaterial(i);
                
                if (material != 0) 
                {
                    std::string type;
                    type = material->GetClassId().GetName();

                    char newMaterialName[32];

                    //// マテリアル毎に分割されるのに合わせてリネーム
                    //sprintf(newMaterialName, "%s%d", name, materialNameCnt);
                    //name = newMaterialName;
                    //++materialNameCnt;

                    // マテリアル解析
                    // LambertかPhongか

                    if (type == "FbxSurfaceLambert") {
                        
                        // Lambertにダウンキャスト
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

                        // Phongにダウンキャスト
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
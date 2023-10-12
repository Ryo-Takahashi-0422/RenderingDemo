#include <stdafx.h>
#include <FBXInfoManager.h>

FBXInfoManager& FBXInfoManager::Instance()
{
    static FBXInfoManager instance;
    return instance;
};

int FBXInfoManager::Init(std::string _modelPath)
{
    FbxManager* manager = nullptr;
    //FbxScene* scene = nullptr;

    modelPath = _modelPath;

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
    ReadFBXFile();

    auto it = indexWithBonesNumAndWeight.begin();
    int meshIndex = 0;
    int vertexIndex = 0;
    int VertexTotalNum = finalVertexDrawOrder[0].second.vertices.size(); // ループ処理内iとの値比較に用いる。この値にiが到達したらmeshIndexをインクリメントして、次のメッシュを読む。その際にそのメッシュのvertex数を加算しておく。
    int lastVertexTotalNum = 0;
    for (int i = 0; i < indexWithBonesNumAndWeight.size(); ++i)
    {
        if (i == VertexTotalNum)
        {
            ++meshIndex;
            lastVertexTotalNum = VertexTotalNum;
            VertexTotalNum += finalVertexDrawOrder[meshIndex].second.vertices.size();
            vertexIndex = 0;
        }
        
        auto it2 = it->second.begin();
        for (int j = 0; j < it->second.size(); ++j)
        {
            if (j < 3)
            {
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_index1[j] = it2->first;
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_weight1[j] = it2->second;
            }
            else
            {
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_index2[j - 3] = it2->first;
                finalVertexDrawOrder[meshIndex].second.vertices[it->first - lastVertexTotalNum].bone_weight2[j - 3] = it2->second;
            }
            ++it2;

        }
        ++it;
        
        ++vertexIndex;
     }

    // マネージャ解放
    // 関連するすべてのオブジェクトが解放される
    scene->Destroy();
    manager->Destroy();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ★複数メッシュは読み込めるがローカル座標で描画するため重複する。読み込みモデルをメッシュ結合することで回避する。
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FBXInfoManager::ReadFBXFile()
{
    FbxMesh* fbxMesh = nullptr;
    std::vector<int> indiceVec; // 右手系インデクス
    std::map<std::string, std::vector<int>> fixedIndiceVec; // 左手系インデクス]DirectX用
    //std::map<std::string, std::vector<LambertInfo>> m_LambertInfo;
    //std::map<std::string, PhongInfo> m_PhongInfo;
    int nameCnt = 0;
    int materialNameCnt = 0;
    int textureNumCnt = 0;

    int meshCount = scene->GetSrcObjectCount<FbxMesh>();

    for(int i = 0; i < meshCount; ++i)
    {
        //Get vertex information(pos, index, uv, normal)
        // 頂点x,y,z座標の抽出及びverticesへの格納
        fbxMesh = scene->GetSrcObject<FbxMesh>(i);
        const char* name = fbxMesh->GetName();

        // UVセット名の取得
        // * 1つのUVセット名のみ対応
        FbxStringList uvSetNameList;
        fbxMesh->GetUVSetNames(uvSetNameList);
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // 頂点座標情報のリストを生成
        //std::vector<std::vector<float>> vertexInfoList;
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
        //std::vector<unsigned int> indicesFiexed4DirectX;
        std::vector<std::array<int, 2>> oldNewIndexPairList;
        for (int polIndex = 0; polIndex < fbxMesh->GetPolygonCount(); polIndex++) // ポリゴン毎のループ
        {
            for (int polVertexIndex = 0; polVertexIndex < fbxMesh->GetPolygonSize(polIndex); polVertexIndex++) // 頂点毎のループ
            {
                // インデックス座標
                auto vertexIndex = fbxMesh->GetPolygonVertex(polIndex, polVertexIndex);
                vertexIndex += meshVertIndexStart;

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

                // インデックス座標を設定。分割されたメッシュのインデックスは、何もしないと番号が0から振り直される。一方、インデックスはメッシュが分割されていようが単一のものだろうが通し番号なので、
                // メッシュを分割する場合は一つ前に読み込んだメッシュのインデックス番号の内、「最大の値 + 1」したものを追加する必要がある。
                indices.push_back(vertexIndex);
            }                
        }

        // 頂点情報を生成
        std::vector<FBXVertex> vertices;
        for (int i = meshVertIndexStart; i < vertexInfoList.size(); i++)
        {
            std::vector<float> vertexInfo = vertexInfoList[i];
            if (vertexInfo.size() > 3)
            {
                vertices.push_back(FBXVertex
                {
                    {
                        vertexInfo[0], vertexInfo[1], vertexInfo[2]
                    },
                    {
                        vertexInfo[3], vertexInfo[4], vertexInfo[5]
                    },
                    {
                        vertexInfo[6], 1.0f - vertexInfo[7] // Blenderから出力した場合、V値は反転させる(モデルが最初に作成されたときのソフト(例：Maya)は関係ない)
                    }
                });
            }
        }

        //// 前回読み込んだメッシュのインデックス番号の内、最大値を抽出する
        //auto iter = std::max_element(indices.begin(), indices.end());
        //size_t index = std::distance(indices.begin(), iter);
        //meshVertIndexStart = indices[index] + 1;

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
            
        materialNameAndVertexInfo[name] = { vertices, indices/*indicesFiexed4DirectX*/ };
        auto itFI = materialNameAndVertexInfo.begin();
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
        finalVertexDrawOrder.resize(nameCnt + 1);
        finalVertexDrawOrder.at(nameCnt).first = name;
        finalVertexDrawOrder.at(nameCnt).second.vertices = vertices;
        finalVertexDrawOrder.at(nameCnt).second.indices = indices;

        // 接空間処理
        int iTangentCnt = fbxMesh->GetElementTangentCount();
        if (iTangentCnt != 0)
        {
            for (int i = 0; i < finalVertexDrawOrder.at(nameCnt).second.indices.size(); ++i)
            {
                ProcessTangent(fbxMesh, name, nameCnt, finalVertexDrawOrder.at(nameCnt).second.indices[i] - meshVertIndexStart);
                ++testCnt;
            }
        }
        testCnt = 0;

        // 前回読み込んだメッシュのインデックス番号の内、最大値を抽出する
        auto iter = std::max_element(indices.begin(), indices.end());
        size_t index = std::distance(indices.begin(), iter);
        meshVertIndexStart = indices[index] + 1;

        // Get material information
        // マテリアル情報元のノード取得
        FbxNode* node = fbxMesh->GetNode();
        if (node == 0) 
        {
            continue;
        }

        // OBBのためローカル座標・角度取得
        localTransition = node->LclTranslation.Get();
        XMFLOAT3 lPos;
        lPos.x = (float)localTransition.mData[0];
        lPos.y = (float)localTransition.mData[1];
        lPos.z = (float)localTransition.mData[2];
        localPosAndRotOfMesh[name].first = lPos;
        localRotation = node->LclRotation.Get();
        XMFLOAT3 lRot;
        lRot.x = (float)localRotation.mData[0];
        lRot.y = (float)localRotation.mData[1];
        lRot.z = (float)localRotation.mData[2];
        localPosAndRotOfMesh[name].second = lRot;


        // マテリアルの数をチェック
        int materialNum = node->GetMaterialCount();
        if (materialNum == 0) 
        {
            continue;
        }

        // マテリアル情報を取得
        FbxSurfaceMaterial* material = node->GetMaterial(0); // ★統合したメッシュでは「i」
                
        if (material != 0) 
        {
            std::string type;
            type = material->GetClassId().GetName();

            // マテリアル解析:LambertかPhongか
            if (type == "FbxSurfaceLambert") {
                assert(!"invalid material.");
                exit(1);

                //// ★Lambert pattern process has no implemention
                //// Lambertにダウンキャスト
                //FbxSurfaceLambert* lambert = (FbxSurfaceLambert*)material;

                //// Diffuse
                //m_LambertInfo[name][i].diffuse[0] = (float)lambert->Diffuse.Get().mData[0];
                //m_LambertInfo[name][i].diffuse[1] = (float)lambert->Diffuse.Get().mData[1];
                //m_LambertInfo[name][i].diffuse[2] = (float)lambert->Diffuse.Get().mData[2];

                //// Ambient
                //m_LambertInfo[name][i].ambient[0] = (float)lambert->Ambient.Get().mData[0];
                //m_LambertInfo[name][i].ambient[1] = (float)lambert->Ambient.Get().mData[1];
                //m_LambertInfo[name][i].ambient[2] = (float)lambert->Ambient.Get().mData[2];

                //// emissive
                //m_LambertInfo[name][i].emissive[0] = (float)lambert->Emissive.Get().mData[0];
                //m_LambertInfo[name][i].emissive[1] = (float)lambert->Emissive.Get().mData[1];
                //m_LambertInfo[name][i].emissive[2] = (float)lambert->Emissive.Get().mData[2];

                //// bump
                //m_LambertInfo[name][i].bump[0] = (float)lambert->Bump.Get().mData[0];
                //m_LambertInfo[name][i].bump[1] = (float)lambert->Bump.Get().mData[1];
                //m_LambertInfo[name][i].bump[2] = (float)lambert->Bump.Get().mData[2];
            }
            else if (type == "FbxSurfacePhong") {

                // Phongにダウンキャスト
                FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

                // copy and order material data
                finalPhongMaterialOrder.resize(nameCnt + 1);
                finalPhongMaterialOrder.at(nameCnt).first = name;

                // Diffuse
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[0] = (float)phong->Diffuse.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[1] = (float)phong->Diffuse.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.diffuse[2] = (float)phong->Diffuse.Get().mData[2];

                // Ambient
                finalPhongMaterialOrder.at(nameCnt).second.ambient[0] = (float)phong->Ambient.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.ambient[1] = (float)phong->Ambient.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.ambient[2] = (float)phong->Ambient.Get().mData[2];

                // emissive
                finalPhongMaterialOrder.at(nameCnt).second.emissive[0] = (float)phong->Emissive.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.emissive[1] = (float)phong->Emissive.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.emissive[2] = (float)phong->Emissive.Get().mData[2];

                // bump
                finalPhongMaterialOrder.at(nameCnt).second.bump[0] = (float)phong->Bump.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.bump[1] = (float)phong->Bump.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.bump[2] = (float)phong->Bump.Get().mData[2];

                // specular
                finalPhongMaterialOrder.at(nameCnt).second.specular[0] = (float)phong->Specular.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.specular[1] = (float)phong->Specular.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.specular[2] = (float)phong->Specular.Get().mData[2];

                // reflectivity
                finalPhongMaterialOrder.at(nameCnt).second.reflection[0] = (float)phong->Reflection.Get().mData[0];
                finalPhongMaterialOrder.at(nameCnt).second.reflection[1] = (float)phong->Reflection.Get().mData[1];
                finalPhongMaterialOrder.at(nameCnt).second.reflection[2] = (float)phong->Reflection.Get().mData[2];

                // shiness
                finalPhongMaterialOrder.at(nameCnt).second.transparency = (float)phong->TransparencyFactor.Get();
            }
        }


        // Get texture infomation
        for (auto type : textureType)
        {
            // ディフューズプロパティを検索
            FbxProperty property = material->FindProperty(/*FbxSurfaceMaterial::sDiffuse*/type);

            // プロパティが持っているレイヤードテクスチャの枚数をチェック
            int layerNum = property.GetSrcObjectCount<FbxLayeredTexture>();

            // レイヤードテクスチャが無ければ通常テクスチャ
            if (layerNum == 0) {
                // 通常テクスチャの枚数をチェック
                int numGeneralTexture = property.GetSrcObjectCount<FbxFileTexture>();

                // 各テクスチャについてテクスチャ情報をゲット
                for (int i = 0; i < numGeneralTexture; ++i) {
                    // i番目のテクスチャオブジェクト取得
                    FbxFileTexture* texture = /*FbxCast<FbxFileTexture>*/property.GetSrcObject<FbxFileTexture>(i);

                    // テクスチャファイルパスを取得（フルパス）
                    const char* filePath = texture->GetFileName();

                    materialAndTexturenameInfo.resize(textureNumCnt);
                    std::pair<std::string, std::string> pair = { name, filePath };
                    materialAndTexturenameInfo.push_back(pair);
                    ++textureNumCnt;

                }
            }
        }


        // Get bone pos and animation matrix
        // スキンの数を取得
        int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);

        // No skin model can't process and occur error
        if (skinCount > 0) 
        {
            // アニメーション情報も取得
            FbxGlobalSettings& globalSettings = scene->GetGlobalSettings();
            FbxTime::EMode timeMode = globalSettings.GetTimeMode();
            FbxTime period;
            period.SetTime(0, 0, 0, 1, 0, timeMode);

            for (int skinNum = 0; skinNum < skinCount; ++skinNum) {
                // skinNum番目のスキンを取得
                FbxStatus* eStatus;
                FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinNum, FbxDeformer::eSkin));

                int cluster_count = skin->GetClusterCount();
                lastMeshIndexNumByCluster = indexWithBonesNumAndWeight.size();
                for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
                {
                    // get informations per bone(cluster index point to bone)
                    FbxCluster* cluster = skin->GetCluster(cluster_index); // bone like "Head", "Neck", etc...
                    int pointNum = cluster->GetControlPointIndicesCount();
                    int* pointAry = cluster->GetControlPointIndices();
                    double* weightAry = cluster->GetControlPointWeights();

                    for (int i = 0; i < pointNum; ++i) {
                        // 頂点インデックスとウェイトを取得
                        int index = pointAry[i]; //////////////////////////
                        index += lastMeshIndexNumByCluster;
                        float weight = (float)weightAry[i];

                        indexWithBonesNumAndWeight[index][cluster_index] = weight; // vertex index(serial number=通し番号), bone index, bone weight set
                    }                                        
                    
                    // ボーンの初期姿勢を取得
                    if (!isBonesInitialPostureMatrixFilled)
                    {
                        fbxsdk::FbxAMatrix init_matrix;
                        cluster->GetTransformLinkMatrix(init_matrix); // 初期姿勢
                        XMVECTOR v0 = { init_matrix[0].mData[0] ,init_matrix[0].mData[1] ,init_matrix[0].mData[2] ,init_matrix[0].mData[3] };
                        XMVECTOR v1 = { init_matrix[1].mData[0] ,init_matrix[1].mData[1] ,init_matrix[1].mData[2] ,init_matrix[1].mData[3] };
                        XMVECTOR v2 = { init_matrix[2].mData[0] ,init_matrix[2].mData[1] ,init_matrix[2].mData[2] ,init_matrix[2].mData[3] };
                        XMVECTOR v3 = { init_matrix[3].mData[0] ,init_matrix[3].mData[1] ,init_matrix[3].mData[2] ,init_matrix[3].mData[3] };
                        bonesInitialPostureMatrix[cluster_index].r[0] = v0;
                        bonesInitialPostureMatrix[cluster_index].r[1] = v1;
                        bonesInitialPostureMatrix[cluster_index].r[2] = v2;
                        bonesInitialPostureMatrix[cluster_index].r[3] = v3;


                        FbxNode* linked_node = cluster->GetLink();
                        FbxArray< FbxString* > takeNameAry;   // 文字列格納配列
                        scene->FillAnimStackNameArray/*FillTakeNameArray*/(takeNameAry);  // テイク名取得
                        int numTake = takeNameAry.GetCount();     // テイク数
                        FbxTime start;
                        FbxTime stop;

                        // "Take" is Skin name and Animation name set like "Armature|Walking", "Armature|Punching", etc.
                        for (int i = 0; i < numTake; ++i)
                        {
                            // 複数アニメーション情報切り替え
                            FbxAnimStack* pStack = scene->GetSrcObject<FbxAnimStack>(i);
                            scene->SetCurrentAnimationStack(pStack);

                            // テイク名からテイク情報を取得
                            FbxTakeInfo* currentTakeInfo = scene->GetTakeInfo(*(takeNameAry[i]));
                            if (currentTakeInfo)
                            {
                                start = currentTakeInfo->mLocalTimeSpan.GetStart();
                                stop = currentTakeInfo->mLocalTimeSpan.GetStop();

                                // 1フレーム時間（period）で割ればフレーム数になる
                                int startFrame = (int)(start.Get() / period.Get());
                                int stopFrame = (int)(stop.Get() / period.Get());
                                for (int j = startFrame; j < stopFrame; ++j)
                                {
                                    FbxMatrix mat;
                                    FbxTime time = start + period * j;
                                    mat = scene->GetAnimationEvaluator()->GetNodeGlobalTransform(linked_node, time);

                                    XMVECTOR v0 = { mat[0].mData[0] ,mat[0].mData[1] ,mat[0].mData[2] ,mat[0].mData[3] };
                                    XMVECTOR v1 = { mat[1].mData[0] ,mat[1].mData[1] ,mat[1].mData[2] ,mat[1].mData[3] };
                                    XMVECTOR v2 = { mat[2].mData[0] ,mat[2].mData[1] ,mat[2].mData[2] ,mat[2].mData[3] };
                                    XMVECTOR v3 = { mat[3].mData[0] ,mat[3].mData[1] ,mat[3].mData[2] ,mat[3].mData[3] };

                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[0] = v0;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[1] = v1;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[2] = v2;
                                    animationNameAndBoneNameWithTranslationMatrix[(std::string)currentTakeInfo->mImportName][cluster_index][j].r[3] = v3;
                                }
                            }
                        }
                    }
                }
                isBonesInitialPostureMatrixFilled = true;
            }

            // 前処理(頂点インデックスとウェイトを取得)によって追加された頂点インデックスはクラスターからは読み取れないため、追加されたインデックスと元となるインデックスのマップから影響を受けるボーン番号およびそのウェイト、接空間情報をコピーしていく。
            auto itAddtionalIndex = addtionalVertexIndexByApplication.begin();
            for (int i = 0; i < addtionalVertexIndexByApplication.size(); ++i)
            {
                for (int j = 0; j < itAddtionalIndex->second.size(); ++j)
                {
                    auto iter = indexWithBonesNumAndWeight.find(itAddtionalIndex->first);
                    if (iter != indexWithBonesNumAndWeight.end())
                    {
                        indexWithBonesNumAndWeight[itAddtionalIndex->second[j]] = iter->second;
                    }
                }
                ++itAddtionalIndex;
            }
        }

        // 分割メッシュのマテリアル名は全て同一となり処理不能となるため、名称末尾に数値を追加していく
        name = node->GetName();
        nameCnt += 1;
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

    int reserveNewIndex = newIndex;
    addtionalVertexIndexByApplication[oldIndex].push_back(reserveNewIndex);
    return newIndex;
}
// vertexInfoに法線、UV座標が設定済かどうか
bool FBXInfoManager::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    if (fabs(vertexInfo[3] - normalVec4[0]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[6] - uvVec2[0]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[4] - normalVec4[1]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[7] - uvVec2[1]) > FLT_EPSILON) return false;
    if (fabs(vertexInfo[5] - normalVec4[2]) > FLT_EPSILON) return false;

    return true;
}

void FBXInfoManager::ProcessTangent(FbxMesh* mesh, std::string materialName, int nameCnt, int indexNum)
{
    if (mesh->GetElementTangentCount() == 0)
    {
        return;
    }

    FbxGeometryElementTangent* tangent = mesh->GetElementTangent(0);
    FbxGeometryElementBinormal* biNormal = mesh->GetElementBinormal(0);
    FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

    switch (tangent->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
        switch (tangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            //const auto vt = tangent->GetDirectArray().GetAt(vertexIndex);
            //outTangent.x = static_cast<float>(vt.mData[0]);
            //outTangent.y = static_cast<float>(vt.mData[1]);
            //outTangent.z = static_cast<float>(vt.mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>( vt.mData[ 3 ] );
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            //const int index = tangent->GetIndexArray().GetAt(vertexIndex);
            //const auto vt = tangent->GetDirectArray().GetAt(index);
            //outTangent.x = static_cast<float>(vt.mData[0]);
            //outTangent.y = static_cast<float>(vt.mData[1]);
            //outTangent.z = static_cast<float>(vt.mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>( vt.mData[ 3 ] );
        }
        break;

        default:
            assert(!"invalid reference.");
            exit(1);
        }
        break;

    case FbxGeometryElement::eByPolygonVertex:
        switch (tangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt].resize(9);
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][0] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][1] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][2] = (static_cast<float>(tangent->GetDirectArray().GetAt(testCnt).mData[2]));

            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][3] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][4] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][5] = (static_cast<float>(biNormal->GetDirectArray().GetAt(testCnt).mData[2]));

            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][6] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[0]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][7] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[1]));
            //indexWithTangentBinormalNormalByMaterialName[materialName][testCnt][8] = (static_cast<float>(vertexNormal->GetDirectArray().GetAt(testCnt).mData[2]));
            

            if (std::find(indexOFTangentBinormalNormalByMaterialName[materialName].begin(), indexOFTangentBinormalNormalByMaterialName[materialName].end(), indexNum) == indexOFTangentBinormalNormalByMaterialName[materialName].end())
            {
                indexOFTangentBinormalNormalByMaterialName[materialName].emplace_back(indexNum);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[0] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[1] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).tangent[2] = static_cast<float>(tangent->GetDirectArray().GetAt(indexNum).mData[2]);

                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[0] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[1] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).biNormal[2] = static_cast<float>(biNormal->GetDirectArray().GetAt(indexNum).mData[2]);

                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[0]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[1]);
                finalVertexDrawOrder[nameCnt].second.vertices.at(indexNum).vNormal[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(indexNum).mData[2]);
            }
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt( vertexCounter ).mData[ 3 ]);            
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            //int index = tangent->GetIndexArray().GetAt(vertexCounter);
            //outTangent.x = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[0]);
            //outTangent.y = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[1]);
            //outTangent.z = static_cast<float>(tangent->GetDirectArray().GetAt(index).mData[2]);
            // FIXME: returns 0, but must be -1 or 1: outTangent.w = static_cast<float>(tangent->GetDirectArray().GetAt( index ).mData[ 3 ]);
        }
        break;

        default:
            assert(!"Invalid reference for tangent.");
            exit(1);
        }
        break;

    case FbxGeometryElement::eNone:
    case FbxGeometryElement::eByPolygon:
    case FbxGeometryElement::eByEdge:
    case FbxGeometryElement::eAllSame:
        std::cerr << "Unhandled mapping mode for tangent.";
        exit(1);
        break;
    }
}
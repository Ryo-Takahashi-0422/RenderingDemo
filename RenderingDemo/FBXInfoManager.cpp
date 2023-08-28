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
    FbxScene* scene = FbxScene::Create(manager, "scene");
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
        enumNodeNamesAndAttributes(root, 0);
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

void FBXInfoManager::enumNodeNamesAndAttributes(FbxNode* node, int indent)
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
            
            // 頂点バッファの取得
            FbxVector4* vertices = fbxMesh->GetControlPoints();
            // インデックスバッファの取得
            int* indices = fbxMesh->GetPolygonVertices();
          
            // 頂点座標の数の取得
            int polygon_vertex_count = fbxMesh->GetPolygonVertexCount();
            vertNum += fbxMesh->GetControlPointsCount();

            // GetPolygonVertexCount => 頂点数
            for (int i = 0; i < polygon_vertex_count; i++)
            {
                VertexInfo vertexInfo = {};
                // インデックスバッファから頂点番号を取得
                int index = indices[i];
                indiceVec.emplace_back(index);

                // 頂点座標リストから座標を取得する
                vertexInfo.pos[0] = vertices[index][0];
                vertexInfo.pos[1] = vertices[index][1];
                vertexInfo.pos[2] = vertices[index][2];

                if (indexAndVertexPos.find(index) == indexAndVertexPos.end())
                {
                    indexAndVertexPos[index][0] = vertices[index][0];
                    indexAndVertexPos[index][1] = vertices[index][1];
                    indexAndVertexPos[index][2] = vertices[index][2];
                }

                // 追加
                m_VertexInfo[name].push_back(vertexInfo);
            }

            // 左手系インデクスに修正
            for (int i = indiceCnt; i < /*fbxMesh->GetPolygonCount()*/indiceVec.size() / 3; ++i)
            {
                // 2 => 1 => 0にしてるのは左手系対策
                fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 2]);
                fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3 + 1]);
                fixedIndiceVec[name].push_back(indiceVec[indiceCnt * 3]);
                ++indiceCnt;
            }
                       
            // 法線リストの取得
            FbxArray<FbxVector4> normals;
            fbxMesh->GetPolygonVertexNormals(normals);
            // 法線設定
            for (int i = 0; i < normals.Size(); i++)
            {
                m_VertexInfo[name][i].normal_vec[0] = normals[i][0];
                m_VertexInfo[name][i].normal_vec[1] = normals[i][1];
                m_VertexInfo[name][i].normal_vec[2] = normals[i][2];
            }

            // uv取得
            int layerCount = fbxMesh->GetLayerCount();   // meshはKFbxMesh
            for (int i = 0; i < layerCount; ++i) 
            {
                FbxLayer* layer = fbxMesh->GetLayer(i);
                FbxLayerElementUV* elem = layer->GetUVs();
                if (elem == 0) 
                {
                    continue;
                }

                // UV情報を取得
                // UVの数・インデックス
                int UVNum = elem->GetDirectArray().GetCount();
                int indexNum = elem->GetIndexArray().GetCount();
                int size = UVNum > indexNum ? UVNum : indexNum;
                XMFLOAT2* buffer = new XMFLOAT2[size];

                // マッピングモード・リファレンスモード別にUV取得
                FbxLayerElement::EMappingMode mappingMode = elem->GetMappingMode();
                FbxLayerElement::EReferenceMode refMode = elem->GetReferenceMode();

                if (mappingMode == FbxLayerElement::eByPolygonVertex)
                {
                    if (refMode == FbxLayerElement::eDirect) 
                    {
                        // 直接取得
                        for (int i = 0; i < size; ++i) 
                        {
                            m_VertexInfo[name][i].uv[0] = (float)elem->GetDirectArray().GetAt(i)[0];
                            m_VertexInfo[name][i].uv[1] = (float)elem->GetDirectArray().GetAt(i)[1];

                            //m_VertexInfo[name][i].uvname = elem->GetName();
                        }
                    }
                    else
                        if (refMode == FbxLayerElement::eIndexToDirect) 
                        {
                            // インデックスから取得
                            for (int i = 0; i < size; ++i) 
                            {
                                int index = elem->GetIndexArray().GetAt(i);
                                m_VertexInfo[name][i].uv[0] = (float)elem->GetDirectArray().GetAt(index)[0];
                                m_VertexInfo[name][i].uv[1] = (float)elem->GetDirectArray().GetAt(index)[1];

                                //m_VertexInfo[name][i].uvname = elem->GetName();
                            }
                        }
                }
            }

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
        enumNodeNamesAndAttributes(node->GetChild(i), indent + 1);
    }

    fbxModelInfo["C:\\Users\\RyoTaka\\Documents\\RenderingDemo-Rebuild\\FBX\\Connan_Walking.fbx"] = fbxPolygonMap;
}
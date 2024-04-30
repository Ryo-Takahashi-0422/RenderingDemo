#include <stdafx.h>
#include <FBXInfoManager.h>

FBXInfoManager& FBXInfoManager::Instance()
{
    static FBXInfoManager instance;
    return instance;
};

int FBXInfoManager::Init(std::string _modelPath)
{
    struct meshName
    {
        char* name;
    };

    const char* fileName = _modelPath.c_str();

    auto fp = fopen(fileName, "rb");
    if (fp == nullptr) {
        //ÉGÉâÅ[èàóù
        assert(0);
        return ERROR_FILE_NOT_FOUND;
    }

    // finalVertexInfoì«Ç›çûÇ›èàóù
    int finalVertexInfoSize = 0;
    unsigned int meshNameSize = 0;
    std::vector<meshName> meshNames;
    int verticesSize;
    int indiceSize;

    fread(&finalVertexInfoSize, sizeof(finalVertexInfoSize), 1, fp);
    meshNames.resize(finalVertexInfoSize);
    finalVertexDrawOrder.resize(meshNames.size());
    for (int i = 0; i < meshNames.size(); ++i)
    {
        fread(&meshNameSize, sizeof(meshNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, meshNameSize, 1, fp);
        meshNames[i].name = tempName;
        //finalVertexDrawOrder[i].first = tempName;

        fread(&verticesSize, sizeof(verticesSize), 1, fp);
        fread(&indiceSize, sizeof(indiceSize), 1, fp);

        std::vector<VertexInfo> tempVertexInfo;
        FBXVertex tempVertex = {};
        unsigned int tempIndex = 0;
        tempVertexInfo.resize(1);

        // vertexì«Ç›çûÇ›
        for (int j = 0; j < verticesSize; ++j)
        {
            fread(&tempVertex, sizeof(tempVertex), 1, fp);
            tempVertexInfo[0].vertices.push_back(tempVertex);
        }

        // indicesì«Ç›çûÇ›
        for (int j = 0; j < indiceSize; ++j)
        {
            fread(&tempIndex, sizeof(tempIndex), 1, fp);
            tempVertexInfo[0].indices.push_back(tempIndex);
        }

        finalVertexDrawOrder[i].first = tempName;
        finalVertexDrawOrder[i].second.vertices = tempVertexInfo[0].vertices;
        finalVertexDrawOrder[i].second.indices = tempVertexInfo[0].indices;
    }

    // finalPhongMAterialInfoì«Ç›çûÇ›
    int finalPhongMaterialOrderSize = 0;
    fread(&finalPhongMaterialOrderSize, sizeof(finalPhongMaterialOrderSize), 1, fp);
    meshNames.clear();
    meshNames.resize(finalPhongMaterialOrderSize);
    finalPhongMaterialOrder.resize(finalPhongMaterialOrderSize);
    for (int i = 0; i < meshNames.size(); ++i)
    {
        fread(&meshNameSize, sizeof(meshNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, meshNameSize, 1, fp);
        meshNames[i].name = tempName;

        PhongInfo tempPhongInfo;
        fread(&tempPhongInfo, sizeof(tempPhongInfo), 1, fp);

        finalPhongMaterialOrder[i].first = tempName;
        finalPhongMaterialOrder[i].second = tempPhongInfo;
    }

    // materialAndTexturenameInfoì«Ç›çûÇ›
    int materialAndTexturenameInfoSize = 0;
    fread(&materialAndTexturenameInfoSize, sizeof(materialAndTexturenameInfoSize), 1, fp);
    materialAndTexturenameInfo.resize(materialAndTexturenameInfoSize);
    int materialNameSize = 0;
    int texturePathSize = 0;
    for (int i = 0; i < materialAndTexturenameInfo.size(); ++i)
    {
        fread(&materialNameSize, sizeof(materialNameSize), 1, fp);
        char* tempMaterialName = (char*)calloc(32, sizeof(char));
        fread(tempMaterialName, materialNameSize, 1, fp);

        fread(&texturePathSize, sizeof(texturePathSize), 1, fp);
        char* tempTexturePath = (char*)calloc(256, sizeof(char));
        fread(tempTexturePath, texturePathSize, 1, fp);

        materialAndTexturenameInfo[i].first = tempMaterialName;
        materialAndTexturenameInfo[i].second = tempTexturePath;
    }

    // localPosAndRotOfMeshì«Ç›çûÇ›
    int localPosAndRotOfMeshSize = 0;
    fread(&localPosAndRotOfMeshSize, sizeof(localPosAndRotOfMeshSize), 1, fp);
    meshNames.clear();
    meshNames.resize(localPosAndRotOfMeshSize);
    for (int i = 0; i < meshNames.size(); ++i)
    {
        fread(&meshNameSize, sizeof(meshNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, meshNameSize, 1, fp);
        meshNames[i].name = tempName;

        XMFLOAT3 tempPos, tempRot;
        fread(&tempPos, sizeof(tempPos), 1, fp);
        fread(&tempRot, sizeof(tempRot), 1, fp);

        localPosAndRotOfMesh[tempName].first = tempPos;
        localPosAndRotOfMesh[tempName].second = tempRot;
    }
       
    // animationNameAndBoneNameWithTranslationMatrixì«Ç›çûÇ›
    int animationNameAndBoneNameWithTranslationMatrixSize = 0;
    fread(&animationNameAndBoneNameWithTranslationMatrixSize, sizeof(animationNameAndBoneNameWithTranslationMatrixSize), 1, fp);
    unsigned int animationNameSize = 0;
    for (int i = 0; i < animationNameAndBoneNameWithTranslationMatrixSize; ++i)
    {
        fread(&animationNameSize, sizeof(animationNameSize), 1, fp);
        char* tempAnimationName = (char*)calloc(32, sizeof(char));
        fread(tempAnimationName, animationNameSize, 1, fp);

        // É{Å[Éìêîäiî[
        int boneNum = 0;
        fread(&boneNum, sizeof(boneNum), 1, fp);

        // ÉtÉåÅ[ÉÄêîäiî[
        int frameNum = 0;
        fread(&frameNum, sizeof(frameNum), 1, fp);

        for (int j = 0; j < boneNum; ++j)
        {
            for (int k = 0; k < frameNum; ++k)
            {
                XMMATRIX matrix;
                fread(&matrix, sizeof(matrix), 1, fp);

                animationNameAndBoneNameWithTranslationMatrix[tempAnimationName][j][k] = matrix;
            }
        }
    }

    // bonesInitialPostureMatrixì«Ç›çûÇ›
    int bonesInitialPostureMatrixSize = 0;
    fread(&bonesInitialPostureMatrixSize, sizeof(bonesInitialPostureMatrixSize), 1, fp);
    for (int i = 0; i < bonesInitialPostureMatrixSize; ++i)
    {
        int boneNum;
        fread(&boneNum, sizeof(boneNum), 1, fp);

        XMMATRIX matrix;
        fread(&matrix, sizeof(matrix), 1, fp);

        bonesInitialPostureMatrix[boneNum] = matrix;
    }

    // OBB
    // vertexListOfOBBì«Ç›çûÇ›èàóù
    int vertexListOfOBBSize = 0;
    unsigned int oBBNameSize = 0;
    //std::vector<meshName> oBBNames;
    int oBBVerticesNum;
    int oBBIndiceNum;
    fread(&vertexListOfOBBSize, sizeof(vertexListOfOBBSize), 1, fp);
    //oBBNames.resize(vertexListOfOBBSize);
    vertexListOfOBB.resize(vertexListOfOBBSize);
    for (int i = 0; i < vertexListOfOBBSize; ++i)
    {
        fread(&oBBNameSize, sizeof(oBBNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, oBBNameSize, 1, fp);
        //oBBNames[i].name = tempName;
        //finalVertexDrawOrder[i].first = tempName;

        fread(&oBBVerticesNum, sizeof(oBBVerticesNum), 1, fp);
        fread(&oBBIndiceNum, sizeof(oBBIndiceNum), 1, fp);

        std::vector<VertexInfo> tempVertexInfo;
        FBXVertex tempVertex = {};
        unsigned int tempIndex = 0;
        tempVertexInfo.resize(1);

        // vertexì«Ç›çûÇ›
        for (int j = 0; j < oBBVerticesNum; ++j)
        {
            fread(&tempVertex, sizeof(tempVertex), 1, fp);
            tempVertexInfo[0].vertices.push_back(tempVertex);
        }

        // indicesì«Ç›çûÇ›
        for (int j = 0; j < oBBIndiceNum; ++j)
        {
            fread(&tempIndex, sizeof(tempIndex), 1, fp);
            tempVertexInfo[0].indices.push_back(tempIndex);
        }

        vertexListOfOBB[i].first = tempName;
        vertexListOfOBB[i].second.vertices = tempVertexInfo[0].vertices;
        vertexListOfOBB[i].second.indices = tempVertexInfo[0].indices;
    }

    // localPosAndRotOfOBBì«Ç›çûÇ›
    int localPosAndRotOfOBBSize = 0;
    fread(&localPosAndRotOfOBBSize, sizeof(localPosAndRotOfOBBSize), 1, fp);
    meshNames.clear();
    meshNames.resize(localPosAndRotOfOBBSize);
    for (int i = 0; i < meshNames.size(); ++i)
    {
        fread(&meshNameSize, sizeof(meshNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, meshNameSize, 1, fp);
        meshNames[i].name = tempName;

        XMFLOAT3 tempPos, tempRot;
        fread(&tempPos, sizeof(tempPos), 1, fp);
        fread(&tempRot, sizeof(tempRot), 1, fp);

        localPosAndRotOfOBB[tempName].first = tempPos;
        localPosAndRotOfOBB[tempName].second = tempRot;
    }

    // OCC
    // vertexListOfOCCì«Ç›çûÇ›èàóù
    int vertexListOfOCCSize = 0;
    unsigned int OCCNameSize = 0;
    //std::vector<meshName> OCCNames;
    int OCCVerticesNum;
    int OCCIndiceNum;
    fread(&vertexListOfOCCSize, sizeof(vertexListOfOCCSize), 1, fp);
    //OCCNames.resize(vertexListOfOCCSize);
    vertexListOfOCC.resize(vertexListOfOCCSize);
    for (int i = 0; i < vertexListOfOCCSize; ++i)
    {
        fread(&OCCNameSize, sizeof(OCCNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, OCCNameSize, 1, fp);
        //OCCNames[i].name = tempName;
        //finalVertexDrawOrder[i].first = tempName;

        fread(&OCCVerticesNum, sizeof(OCCVerticesNum), 1, fp);
        fread(&OCCIndiceNum, sizeof(OCCIndiceNum), 1, fp);

        std::vector<VertexInfo> tempVertexInfo;
        FBXVertex tempVertex = {};
        unsigned int tempIndex = 0;
        tempVertexInfo.resize(1);

        // vertexì«Ç›çûÇ›
        for (int j = 0; j < OCCVerticesNum; ++j)
        {
            fread(&tempVertex, sizeof(tempVertex), 1, fp);
            tempVertexInfo[0].vertices.push_back(tempVertex);
        }

        // indicesì«Ç›çûÇ›
        for (int j = 0; j < OCCIndiceNum; ++j)
        {
            fread(&tempIndex, sizeof(tempIndex), 1, fp);
            tempVertexInfo[0].indices.push_back(tempIndex);
        }

        vertexListOfOCC[i].first = tempName;
        vertexListOfOCC[i].second.vertices = tempVertexInfo[0].vertices;
        vertexListOfOCC[i].second.indices = tempVertexInfo[0].indices;
    }

    // localPosAndRotOfOCCì«Ç›çûÇ›
    int localPosAndRotOfOCCSize = 0;
    fread(&localPosAndRotOfOCCSize, sizeof(localPosAndRotOfOCCSize), 1, fp);
    meshNames.clear();
    meshNames.resize(localPosAndRotOfOCCSize);
    for (int i = 0; i < meshNames.size(); ++i)
    {
        fread(&meshNameSize, sizeof(meshNameSize), 1, fp);
        char* tempName = (char*)calloc(32, sizeof(char));
        fread(tempName, meshNameSize, 1, fp);
        meshNames[i].name = tempName;

        XMFLOAT3 tempPos, tempRot;
        fread(&tempPos, sizeof(tempPos), 1, fp);
        fread(&tempRot, sizeof(tempRot), 1, fp);

        localPosAndRotOfOCC[tempName].first = tempPos;
        localPosAndRotOfOCC[tempName].second = tempRot;
    }
    
    fclose(fp);
    return 0;
}

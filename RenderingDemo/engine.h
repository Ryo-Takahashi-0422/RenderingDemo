#pragma once
#define NOMINMAX
#define PI 3.14159265

#include <DirectXTex.h>
#include <Windows.h>
#include<tchar.h>
//#ifdef _DEBUG
//#include <pix3.h>
#include <iostream>
//#endif // _DEBUG
#include <vector>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3dcompiler.h>//シェーダーコンパイルに必要
#include <d3dx12.h>
#include <string.h>
#include <map>
#include <sys/stat.h>
#include <wrl.h>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <ReadData.h>
#include <stdlib.h>
#include <fstream>

//eigen
#define _USE_MATH_DEFINES
#include <iostream>  // たぶん絶対必要
#include <Eigen/Dense>
#include <Eigen/Geometry> //EigenのGeometry関連の関数を使う場合，これが必要
#include <math.h> // sin cos とか
using namespace Eigen;

// Effekseer
#include <Effekseer.h>
#include <EffekseerRendererDx12.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;

// DirectXTK
#include <SpriteFont.h> // 文字列表示に必要
#include <ResourceUploadBatch.h> // DirectXTK関連のリソース使用に必要
#pragma comment(lib, "DirectXTK12.lib")

//#include <AppD3DX12.h>
#include <InputLayoutBase.h>
#include <VertexInputLayout.h>
#include <CreateD3DX12ResourceBuffer.h>
#include <Utility.h>
#include <PMDMaterialInfo.h>
#include <PrepareRenderingWindow.h>
#include <SetRootSignatureBase.h>
#include <SetRootSignature.h>
#include <SettingShaderCompile.h>
#include <VMDMotionInfo.h>
#include <PMDActor.h>
#include <IGraphicsPipelineSetting.h>
#include <GraphicsPipelineSetting.h>
#include <TextureLoader.h>
#include <BufferHeapCreator.h>

#include <MappingExecuter.h>
#include <ViewCreator.h>
#include <sstream>
//#include <AppD3DX12.h>

#include <PeraPolygon.h> // ﾏﾙﾁﾊﾟｽテスト用
#include <PeraLayout.h>
#include <PeraGraphicsPipelineSetting.h>
#include <PeraSetRootSignature.h>
#include <BufferShaderCompile.h>

#include <LightMapShaderCompile.h>
#include <LightMapGraphicsPipelineSetting.h>

#include <BloomShaderCompile.h>

#include <AOGraphicsPipelineSetting.h>
#include <AOShaderCompile.h>

#include <SettingImgui.h>

// Rebuild
#include <Camera.h>
#include <FBXInfoManager.h>
#include <ResourceManager.h>
#include <TextureTransporter.h>
#include <Input.h>
#include <CollisionManager.h>
#include <CollisionRootSignature.h>
#include <ColliderGraphicsPipelineSetting.h>

// Sky
#include <Sky.h>
#include <ParticipatingMedia.h>
#include <ShadowFactor.h>
#include <Sun.h>
#include <SkyLUT.h>
#include <Shadow.h>

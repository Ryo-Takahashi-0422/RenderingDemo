#pragma once
#define NOMINMAX
#define PI 3.14159265

#include <DirectXTex.h>
#include <Windows.h>
#include<tchar.h>
#ifdef _DEBUG
#include <pix3.h>
#endif
#include <iostream>

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
#include <stdlib.h>
#include <fstream>

// Effekseer
//#include <Effekseer.h>
//#include <EffekseerRendererDx12.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;

// DirectXTK
//#include <SpriteFont.h> // 文字列表示に必要
#include <ResourceUploadBatch.h> // DirectXTK関連のリソース使用に必要
//#pragma comment(lib, "DirectXTK12.lib")

#include <VertexInputLayout.h>
#include <CreateD3DX12ResourceBuffer.h>
#include <Utility.h>
#include <PrepareRenderingWindow.h>
#include <SetRootSignature.h>
#include <SettingShaderCompile.h>

#include <GraphicsPipelineSetting.h>
#include <TextureLoader.h>

#include <PeraPolygon.h>
#include <PeraLayout.h>
#include <PeraGraphicsPipelineSetting.h>
#include <PeraSetRootSignature.h>

// imgui (#define...がないと怒られる。書かない場合、settingImgui上段の""includeで読み込み可能)
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

#include <SettingImgui.h>

// Rebuild
#include <Camera.h>
#include <FBXInfoManager.h>
#include <ResourceManager.h>
#include <TextureTransporter.h>
#include <Input.h>
#include <CollisionRootSignature.h>
#include <ColliderGraphicsPipelineSetting.h>
#include <CollisionManager.h>
#include <OBBManager.h>
#include <Blur.h>
#include <ComputeBlur.h>
#include <Integration.h>
#include <DepthMapIntegration.h>
#include <CalculateSSAO.h>
#include <PreFxaa.h>
#include <OcclusionCullingManager.h>

// Sky
#include <Sky.h>
#include <ParticipatingMedia.h>
#include <ShadowFactor.h>
#include <Sun.h>
#include <SkyLUT.h>
#include <Shadow.h>
#include <Air.h>
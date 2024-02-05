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
#include <d3dcompiler.h>//�V�F�[�_�[�R���p�C���ɕK�v
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
#include <iostream>  // ���Ԃ��ΕK�v
#include <Eigen/Dense>
#include <Eigen/Geometry> //Eigen��Geometry�֘A�̊֐����g���ꍇ�C���ꂪ�K�v
#include <math.h> // sin cos �Ƃ�
using namespace Eigen;

// Effekseer
#include <Effekseer.h>
#include <EffekseerRendererDx12.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;

// DirectXTK
#include <SpriteFont.h> // ������\���ɕK�v
#include <ResourceUploadBatch.h> // DirectXTK�֘A�̃��\�[�X�g�p�ɕK�v
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

#include <PeraPolygon.h> // ����߽�e�X�g�p
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

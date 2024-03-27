#pragma once
#include <ParticipatingMedia.h>

struct PostSetting
{
	bool isFoV;
	float bloomCol[3];
	bool isSSAO;
	float dummy; // to alignment
	bool isBloom;
};

class SettingImgui
{
private:
	ComPtr<ID3D12DescriptorHeap> imguiSRVHeap = nullptr; // imguiのSRV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> imguiRTVHeap = nullptr; // imguiのRTV用ディスクリプタヒープ
	ComPtr<ID3D12Resource> renderingResource = nullptr; // imgui用バッファー

	HRESULT CreateSRVDHeap(ComPtr<ID3D12Device> _dev);
	HRESULT CreateRTVDHeap(ComPtr<ID3D12Device> _dev);
	HRESULT CreateRenderResource(ComPtr<ID3D12Device> _dev, int width, int height);
	void CreateRTV(ComPtr<ID3D12Device> _dev);
	bool blnResult;
	float fovValueExp;
	float bgColorExp[4];
	float lightVecExp[3];
	float bloomExp[3];
	bool isFoV;
	bool isSSAO;
	bool isSelfShadowOn;
	bool isBloomOn;
	bool isEffectOn;

	PostSetting* mappedPostSetting = nullptr;

	// sun
	float sunAngleX = 340.0f;
	float sunAngleY = 70.0f;

	// air param
	ParticipatingMedia m_Media;
	ParticipatingMedia* exp_Media = nullptr;
	bool isAirParamChanged = false;

	// skyLUT param
	ParticipatingMedia sky_Media;
	ParticipatingMedia* exp_skyMedia = nullptr;
	bool isSkyParamChanged = false;

	//sky
	int skyResX = 64;
	int skyResY = 64;
	bool isSkyResChanged = false;

	// skyLUT
	int skyLUTResX = 1024;
	int skyLUTResY = 1024;
	bool isSkyLUTResChanged = false;

	// shadow resolutuion
	int shadowResX = 4096;
	int shadowResY = 4096;
	bool isShadowResChanged = false;

	// shadowFavtor
	int shadowFactorResX = 1024;
	int shadowFactorResY = 1024;
	bool isShadowFactorResChanged = false;
	std::vector<std::string> itemList = { "16", "512", "1024" };

	// draw
	bool sponzaBox = true;
	bool sponzaDraw = true;

	bool colliderBox = false;
	bool colliderDraw = false;

	bool airBox = true;
	bool airDraw = true;

	bool ssaoBox = true;
	bool ssaoDraw = true;

	bool fovBox = true;
	bool fovDraw = true;

	// fps
	bool isFpsChanged = false;
	float fps = 120.0f;
	bool fps30 = false;
	bool fps60 = false;
	bool fps90 = false;
	bool fps120 = true;

public:
	// マルチパスSRV用ディスクリプタヒープの作成
	HRESULT Init(ComPtr<ID3D12Device> _dev,	PrepareRenderingWindow* pRWindow);
	~SettingImgui();
	void DrawImGUI(ComPtr<ID3D12Device> _dev, ComPtr<ID3D12GraphicsCommandList> _cmdList);

	float GetFovValue() { return fovValueExp; };
	float GetBGColor(int num) { return bgColorExp[num]; };
	float GetLightVector(int num) { return lightVecExp[num]; };
	float GetBloomValue(int num) { return bloomExp[num]; };
	bool GetFoVBool() { return isFoV; };
	bool GetSSAOBool() { return isSSAO; };
	bool GetShadowmapOnOffBool() { return isSelfShadowOn; };
	bool GetBloomOnOffBool() { return isBloomOn; };
	bool GetEffectOnOffBool() { return isEffectOn; };
	size_t GetPostSettingSize() { return sizeof(PostSetting); };

	// sun
	float GetSunAngleX() { return sunAngleX; };
	float GetSunAngleY() { return sunAngleY; };

	// air param
	bool GetIsAirParamChanged() { return isAirParamChanged; };
	ParticipatingMedia* GetParticipatingMediaParam() { return exp_Media; };

	// skyLUT param
	bool GetIsSkyLUTParamChanged() { return isSkyParamChanged; };
	ParticipatingMedia* GetPMediaParam4SkyLUT() { return exp_skyMedia; };

	// sky resolutiuon
	bool GetIsSkyResolutionChanged() { return isSkyResChanged; };
	int GetSkyResX() { return skyResX; };
	int GetSkyResY() { return skyResY; };

	// skyLUT resolutiuon
	bool GetIsSkyLUTResolutionChanged() { return isSkyLUTResChanged; };
	int GetSkyLUTResX() { return skyLUTResX; };
	int GetSkyLUTResY() { return skyLUTResY; };

	// shadow resolutiuon
	bool GetIsShadowResolutionChanged() { return isShadowResChanged; };
	bool is4K = true;
	bool is2K = false;
	bool is1K = false;
	int GetShadowResX() { return shadowResX; };
	int GetShadowResY() { return shadowResY; };

	// shadowFactor
	bool GetIsShadowFactorResolutionChanged() { return isShadowFactorResChanged; };
	int GetShadowFactorResX() { return shadowFactorResX; };
	int GetShadowFactorResY() { return shadowFactorResY; };

	//Draw
	bool GetSponzaBoxChanged() { return sponzaDraw; };
	bool GetCollisionBoxChanged() { return colliderDraw; };
	bool GetAirBoxChanged() { return airDraw; };
	bool GetSSAOBoxChanged() { return ssaoDraw; };
	bool GetFOVBoxChanged() { return fovDraw; };

	// fps
	bool GetIsFpsChanged() { return isFpsChanged; };
	float GetFPS() { return fps; };

	ComPtr<ID3D12Resource> GetImguiRenderingResource() { return renderingResource; };

	void ChangeResolution(ComPtr<ID3D12Device> _dev, PrepareRenderingWindow* pRWindow, UINT _width, UINT _height);
};
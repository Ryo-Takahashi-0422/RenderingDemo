#pragma once

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

	float sunAngleX = 0.0f;
	float sunAngleY = 60.0f;

	int skyResX = 1024;
	int skyResY = 1024;
	bool isSkyResChanged = false;

	int skyLUTResX = 1024;
	int skyLUTResY = 1024;
	bool isSkyLUTResChanged = false;

public:
	// マルチパスSRV用ディスクリプタヒープの作成
	HRESULT Init(ComPtr<ID3D12Device> _dev,	PrepareRenderingWindow* pRWindow);

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

	// sky
	bool GetIsSkyResolutionChanged() { return isSkyResChanged; };
	int GetSkyResX() { return skyResX; };
	int GetSkyResY() { return skyResY; };

	// skyLUT
	bool GetIsSkyLUTResolutionChanged() { return isSkyLUTResChanged; };
	int GetSkyLUTResX() { return skyLUTResX; };
	int GetSkyLUTResY() { return skyLUTResY; };

	ComPtr<ID3D12Resource> GetImguiRenderingResource() { return renderingResource; };
};
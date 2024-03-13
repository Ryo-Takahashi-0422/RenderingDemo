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
	ComPtr<ID3D12DescriptorHeap> imguiSRVHeap = nullptr; // imgui��SRV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> imguiRTVHeap = nullptr; // imgui��RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> renderingResource = nullptr; // imgui�p�o�b�t�@�[

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
	float sunAngleX = 360.0f;
	float sunAngleY = 90.0f;

	//sky
	int skyResX = 64;
	int skyResY = 64;
	bool isSkyResChanged = false;

	// skyLUT
	int skyLUTResX = 1024;
	int skyLUTResY = 1024;
	bool isSkyLUTResChanged = false;

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

public:
	// �}���`�p�XSRV�p�f�B�X�N���v�^�q�[�v�̍쐬
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

	// shadowFactor
	bool GetIsShadowFactorResolutionChanged() { return isShadowFactorResChanged; };
	int GetShadowFactorResX() { return shadowFactorResX; };
	int GetShadowFactorResY() { return shadowFactorResY; };

	//Draw
	bool GetSponzaBoxChanged() { return sponzaDraw; };
	bool GetCollisionBoxChanged() { return colliderDraw; };

	ComPtr<ID3D12Resource> GetImguiRenderingResource() { return renderingResource; };
};
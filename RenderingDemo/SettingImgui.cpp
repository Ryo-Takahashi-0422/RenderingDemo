#include <stdafx.h>
#include <SettingImgui.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

//float SettingImgui::fov;

HRESULT SettingImgui::Init
(
	ComPtr<ID3D12Device> _dev,
	PrepareRenderingWindow* pRWindow
)
{
	CreateSRVDHeap(_dev);
	CreateRTVDHeap(_dev);
	CreateRenderResource(_dev, pRWindow->GetWindowWidth(), pRWindow->GetWindowHeight());
	CreateRTV(_dev);
	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();


	blnResult = ImGui_ImplWin32_Init(pRWindow->GetHWND());
	if (!blnResult)
	{
		assert(0);
		return E_FAIL;
	}

	blnResult = ImGui_ImplDX12_Init
	(
		_dev.Get(),
		3,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		imguiSRVHeap.Get(),
		imguiSRVHeap->GetCPUDescriptorHandleForHeapStart(),
		imguiSRVHeap->GetGPUDescriptorHandleForHeapStart()
	);
	if (!blnResult)
	{
		assert(0);
		return E_FAIL;
	}

	return S_OK;
}

void SettingImgui::DrawImGUI(ComPtr<ID3D12Device> _dev, ComPtr<ID3D12GraphicsCommandList> _cmdList)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Rendering Test Menu");
	ImGui::SetWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);
	//ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

	//ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
	if (ImGui::TreeNode("Sun"))
	{
		ImGui::SliderFloat("Sun Angle X", &sunAngleX, 0, 360);
		ImGui::SliderFloat("Sun Angle Y", &sunAngleY, 0, 90);
		//ImGui::InputFloat("Sun Intensity", &sunIntensity_);
		//ImGui::InputFloat("Sun Disk Size", &sunDiskSize_, 0, 0, 8);
		//if (ImGui::InputInt("Sun Disk Segments", &sunDiskSegments_))
		//{
		//	sunDiskSegments_ = (std::max)(sunDiskSegments_, 1);
		//	sunRenderer_.setSunDiskSegments(sunDiskSegments_);
		//}
		//ImGui::ColorEdit3("Sun Color", &sunColor_.x);
		ImGui::TreePop();
	}


	if (ImGui::TreeNode("Sky"))
	{
		isSkyResChanged = false;

		if (ImGui::SliderInt("Sky Resolution X", &skyResX, 1, 1024))
		{
			isSkyResChanged = true;
		}
		if (ImGui::SliderInt("Sky Resolution Y", &skyResY, 1, 1024))
		{
			isSkyResChanged = true;
		}
		
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Sky LUT"))
	{
		isSkyLUTResChanged = false;

		if (ImGui::SliderInt("SkyLUT Resolution X", &skyLUTResX, 1, 1024))
		{
			isSkyLUTResChanged = true;
		}
		if (ImGui::SliderInt("SkyLUT Resolution Y", &skyLUTResY, 1, 1024))
		{
			isSkyLUTResChanged = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Draw"))
	{
		if (ImGui::Checkbox("Sponza", &sponzaBox))
		{
			sponzaDraw = sponzaBox;
		}
		if (ImGui::Checkbox("Collider", &colliderBox))
		{
			colliderDraw = colliderBox;
		}
		if (ImGui::Checkbox("Air", &airBox))
		{
			airDraw = airBox;
		}
		if (ImGui::Checkbox("SSAO", &ssaoBox))
		{
			ssaoDraw = ssaoBox;
		}
		if (ImGui::Checkbox("FOV", &fovBox))
		{
			fovDraw = fovBox;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("fps"))
	{
		isFpsChanged = false;

		if (ImGui::Checkbox("120", &fps0))
		{
			fps = 120;
			fps1 = false;
			isFpsChanged = true;
		}
		if (ImGui::Checkbox("60", &fps1))
		{
			fps = 60;
			fps0 = false;
			isFpsChanged = true;
		}
		ImGui::TreePop();
	}

	// Shadow Factorの解像度変更はプログラムがクラッシュするため一時封印
	//if (ImGui::TreeNode("ShadowFactor"))
	//{
	//	isShadowFactorResChanged = false;

	//	//if (ImGui::SliderInt("ShadowFactor Resolution X", &shadowFactorResX, 16, 1024))
	//	//{
	//	//	isShadowFactorResChanged = true;
	//	//}
	//	//if (ImGui::SliderInt("ShadowFactor Resolution Y", &shadowFactorResY, 16, 1024))
	//	//{
	//	//	isShadowFactorResChanged = true;
	//	//}

	//	/*std::vector<std::string> itemList = {"16", "512", "1024"};*/
	//	static const char* s_currentItem = nullptr;
	//	if (ImGui::BeginCombo("Combo", s_currentItem)) {
	//		for (int i = 0; i < itemList.size(); ++i) {
	//			const bool is_selected = (s_currentItem == itemList[i].c_str());
	//			if (ImGui::Selectable(itemList[i].c_str(), is_selected))
	//				s_currentItem = itemList[i].c_str();
	//			if (is_selected)
	//			{
	//				shadowFactorResY = std::atoi(itemList[i].c_str());
	//				isShadowFactorResChanged = true;
	//			}
	//		}
	//		ImGui::EndCombo();
	//	}

	//	ImGui::TreePop();
	//}


	//static bool blnFoV = false;
	//ImGui::Checkbox("Field of View on/off", &blnFoV);
	//isFoV = blnFoV;

	//static bool blnSSAO = false;
	//ImGui::Checkbox("SSAO on/off", &blnSSAO);
	//isSSAO = blnSSAO;

	//static bool blnShadowmap = false;
	//ImGui::Checkbox("Self Shadow on/off", &blnShadowmap);
	//isSelfShadowOn = blnShadowmap;

	//static bool blnBloom = false;
	//ImGui::Checkbox("Bloom on/off", &blnBloom);
	//isBloomOn = blnBloom;

	//static bool blnEffect = false;
	//ImGui::Checkbox("Effect on/off", &blnEffect);
	//isEffectOn = blnEffect;

	//constexpr float pi = 3.141592653589f;
	//static float fov = XM_PIDIV2;
	//ImGui::SliderFloat("Field Of View", &fov, pi / 6.0f, pi * 5.0f / 6.0f);
	//fovValueExp = fov;

	//static float lightVec[3] = { -1.0f, 1.0f, -0.5f };
	//// lightVec.x
	//ImGui::SliderFloat("Light Vector.x", &lightVec[0], 1.0f, -1.0f);
	//// lightVec.y
	//ImGui::SliderFloat("Light Vector.y", &lightVec[1], 1.0f, -1.0f);
	//// lightVec.y
	//ImGui::SliderFloat("Light Vector.z", &lightVec[2], 1.0f, -1.0f);
	//for (int i = 0; i < 3; ++i)
	//{
	//	lightVecExp[i] = lightVec[i];
	//}

	//static float bgCol[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	//ImGui::ColorPicker4("BackGround Color", bgCol, ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel |
	//	ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
	//for (int i = 0; i < 4; ++i)
	//{
	//	bgColorExp[i] = bgCol[i];
	//}

	//static float bloomCol[3] = {};
	//ImGui::ColorPicker3("bloom color", bloomCol/*, ImGuiColorEditFlags_::ImGuiColorEditFlags_InputRGB*/);
	//for (int i = 0; i < 3; ++i)
	//{
	//	bloomExp[i] = bloomCol[i];
	//}

	ImGui::End();

	ImGui::Render();


	D3D12_RESOURCE_BARRIER barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderingResource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &barrierDesc);

	//auto cmdList = _cmdList;
	auto handle = imguiRTVHeap->GetCPUDescriptorHandleForHeapStart();
	//handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * 7;
	const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_cmdList->ClearRenderTargetView(handle, clear_color_with_alpha, 0, nullptr);
	_cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);
	_cmdList->SetDescriptorHeaps(1, imguiSRVHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _cmdList.Get());

	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	_cmdList->ResourceBarrier(1, &barrierDesc);
}

HRESULT SettingImgui::CreateSRVDHeap(ComPtr<ID3D12Device> _dev)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	return _dev->CreateDescriptorHeap
	(
		&desc, IID_PPV_ARGS(imguiSRVHeap.ReleaseAndGetAddressOf())
	);
}

HRESULT SettingImgui::CreateRTVDHeap(ComPtr<ID3D12Device> _dev)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = /*D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE*/D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	return _dev->CreateDescriptorHeap
	(
		&desc, IID_PPV_ARGS(imguiRTVHeap.ReleaseAndGetAddressOf())
	);
}

HRESULT SettingImgui::CreateRenderResource(ComPtr<ID3D12Device> _dev, int width, int height)
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	auto depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = width;
	resDesc.Height = height;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	auto result = _dev->CreateCommittedResource
	(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(renderingResource.ReleaseAndGetAddressOf())
	);

	return result;
}

void SettingImgui::CreateRTV(ComPtr<ID3D12Device> _dev)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvViewDesc = {};
	rtvViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto handle = imguiRTVHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateRenderTargetView
	(
		renderingResource.Get(),
		&rtvViewDesc,
		handle
	);
}
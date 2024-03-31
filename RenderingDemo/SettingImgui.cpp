#include <stdafx.h>
#include <SettingImgui.h>
//#include "imgui.h"
//#include "imgui_impl_win32.h"
//#include "imgui_impl_dx12.h"

SettingImgui::~SettingImgui()
{
	mappedPostSetting = nullptr;
	exp_Media = nullptr;
	exp_skyMedia = nullptr;
}

HRESULT SettingImgui::Init
(
	ComPtr<ID3D12Device> _dev,
	PrepareRenderingWindow* pRWindow
)
{
	//m_Media = new ParticipatingMedia();
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

	exp_Media = new ParticipatingMedia;
	exp_Media->mieScattering = 4.7f;
	exp_Media->asymmetryParameter = 0.0f;
	exp_Media->altitudeOfRayleigh = 0.0f;
	exp_Media->altitudeOfMie = 3.5f;
	//exp_Media->rayleighScattering = m_Media.rayleighScattering;

	exp_skyMedia = new ParticipatingMedia;
	//exp_skyMedia->rayleighScattering = m_Media.rayleighScattering;

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
	if (ImGui::TreeNode("Sun Angle"))
	{
		ImGui::SliderFloat("Sun Angle X", &sunAngleX, 0, 360);
		ImGui::SliderFloat("Sun Angle Y", &sunAngleY, 0, 90);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Air Parameter"))
	{
		isAirParamChanged = false;

		isAirParamChanged |= ImGui::SliderFloat3("rayleighScattering", &exp_Media->rayleighScattering.x, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("mieScattering", &exp_Media->mieScattering, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("mieAbsorption", &exp_Media->mieAbsorption, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat3("ozoneAbsorption", &exp_Media->ozoneAbsorption.x, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("asymmetryParameter", &exp_Media->asymmetryParameter, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("altitudeOfRayleigh", &exp_Media->altitudeOfRayleigh, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("altitudeOfMie", &exp_Media->altitudeOfMie, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("halfWidthOfOzone", &exp_Media->halfWidthOfOzone, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("altitudeOfOzone", &exp_Media->altitudeOfOzone, 0, 100);
		isAirParamChanged |= ImGui::SliderFloat("groundRadius", &exp_Media->groundRadius, 0, 100000);
		isAirParamChanged |= ImGui::SliderFloat("atomosphereRadius", &exp_Media->atomosphereRadius, 0, 100000);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SkyLUT Parameter"))
	{
		isSkyParamChanged = false;

		isSkyParamChanged |= ImGui::SliderFloat3("rayleighScattering", &exp_skyMedia->rayleighScattering.x, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("mieScattering", &exp_skyMedia->mieScattering, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("mieAbsorption", &exp_skyMedia->mieAbsorption, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat3("ozoneAbsorption", &exp_skyMedia->ozoneAbsorption.x, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("asymmetryParameter", &exp_skyMedia->asymmetryParameter, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("altitudeOfRayleigh", &exp_skyMedia->altitudeOfRayleigh, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("altitudeOfMie", &exp_skyMedia->altitudeOfMie, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("halfWidthOfOzone", &exp_skyMedia->halfWidthOfOzone, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("altitudeOfOzone", &exp_skyMedia->altitudeOfOzone, 0, 100);
		isSkyParamChanged |= ImGui::SliderFloat("groundRadius", &exp_skyMedia->groundRadius, 0, 100000);
		isSkyParamChanged |= ImGui::SliderFloat("atomosphereRadius", &exp_skyMedia->atomosphereRadius, 0, 100000);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Sky Resolution"))
	{
		isSkyResChanged = false;

		isSkyResChanged |= ImGui::SliderInt("Sky Resolution X", &skyResX, 1, 1024);
		isSkyResChanged |= ImGui::SliderInt("Sky Resolution Y", &skyResY, 1, 1024);
		
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Sky LUT Resolution"))
	{
		isSkyLUTResChanged = false;

		isSkyLUTResChanged |= ImGui::SliderInt("SkyLUT Resolution X", &skyLUTResX, 1, 1024);
		isSkyLUTResChanged |= ImGui::SliderInt("SkyLUT Resolution Y", &skyLUTResY, 1, 1024);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Shadow Resolution"))
	{
		isShadowResChanged = false;

		if (ImGui::Checkbox("4096*4096", &is4K))
		{
			is1K = false;
			is2K = false;
			is4K = true;
			isShadowResChanged = true;
			shadowResX = 4096;
			shadowResY = 4096;
		}
		if (ImGui::Checkbox("2048*2048", &is2K))
		{
			is1K = false;
			is2K = true;
			is4K = false;
			isShadowResChanged = true;
			shadowResX = 2048;
			shadowResY = 2048;
		}
		if (ImGui::Checkbox("1024*1024", &is1K))
		{
			is1K = true;
			is2K = false;
			is4K = false;
			isShadowResChanged = true;
			shadowResX = 1024;
			shadowResY = 1024;
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

	if (ImGui::TreeNode("Max FPS"))
	{
		isFpsChanged = false;

		if (ImGui::Checkbox("30", &fps30))
		{
			fps = 30.0f;
			fps30 = true;
			fps60 = false;
			fps90 = false;
			fps120 = false;
			isFpsChanged = true;
		}

		if (ImGui::Checkbox("60", &fps60))
		{
			fps = 60.0f;
			fps30 = false;
			fps60 = true;
			fps90 = false;
			fps120 = false;
			isFpsChanged = true;
		}

		if (ImGui::Checkbox("90", &fps90))
		{
			fps = 90.0f;
			fps30 = false;
			fps60 = false;
			fps90 = true;
			fps120 = false;
			isFpsChanged = true;
		}

		if (ImGui::Checkbox("120", &fps120))
		{
			fps = 120.0f;
			fps30 = false;
			fps60 = false;
			fps90 = false;
			fps120 = true;
			isFpsChanged = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Anti Aliasing"))
	{
		isAAChanged = false;

		if (ImGui::Checkbox("FXAA", &fxaaBox))
		{
			fxaaDraw = fxaaBox;
			isAAChanged = true;
		}

		ImGui::TreePop();
	}

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

void SettingImgui::ChangeResolution(ComPtr<ID3D12Device> _dev, PrepareRenderingWindow* pRWindow, UINT _width, UINT _height)
{

	imguiSRVHeap.Reset();
	imguiRTVHeap.Reset();
	renderingResource.Reset();

	CreateSRVDHeap(_dev);
	CreateRTVDHeap(_dev);
	if (_width > _height)
	{
		CreateRenderResource(_dev, _height, _height);
	}
	else if (_width < _height)
	{
		CreateRenderResource(_dev, _width, _width);
	}
	else
	{
		CreateRenderResource(_dev, _width, _height);
	}
	//CreateRenderResource(_dev, _width, _height);
	CreateRTV(_dev);

	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX12_Shutdown();

	blnResult = ImGui_ImplWin32_Init(pRWindow->GetHWND());
	if (!blnResult)
	{
		assert(0);
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
	}
}
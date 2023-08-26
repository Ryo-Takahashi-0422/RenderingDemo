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
	CreateSRVDHeap4Imgui(_dev);
	CreateRTVDHeap4Imgui(_dev);
	CreateBuff4Imgui(_dev);
	//CreateRTV4Imgui(_dev);
	
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

void SettingImgui::DrawDateOfImGUI(
	ComPtr<ID3D12Device> _dev,
	ComPtr<ID3D12GraphicsCommandList> _cmdList,
	ComPtr<ID3D12Resource>/*std::vector<ComPtr<ID3D12Resource>>*/ pResoures,
	BufferHeapCreator*/*std::vector<BufferHeapCreator*>*/ bufferHeapCreator,
	UINT backBufferIndex)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Rendering Test Menu");
	ImGui::SetWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);
	//ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

	static bool blnFoV = false;
	ImGui::Checkbox("Field of View on/off", &blnFoV);
	isFoV = blnFoV;

	static bool blnSSAO = false;
	ImGui::Checkbox("SSAO on/off", &blnSSAO);
	isSSAO = blnSSAO;

	static bool blnShadowmap = false;
	ImGui::Checkbox("Self Shadow on/off", &blnShadowmap);
	isSelfShadowOn = blnShadowmap;

	static bool blnBloom = false;
	ImGui::Checkbox("Bloom on/off", &blnBloom);
	isBloomOn = blnBloom;

	static bool blnEffect = false;
	ImGui::Checkbox("Effect on/off", &blnEffect);
	isEffectOn = blnEffect;

	constexpr float pi = 3.141592653589f;
	static float fov = XM_PIDIV2;
	ImGui::SliderFloat("Field Of View", &fov, pi / 6.0f, pi * 5.0f / 6.0f);
	fovValueExp = fov;

	static float lightVec[3] = { -1.0f, 1.0f, -0.5f };
	// lightVec.x
	ImGui::SliderFloat("Light Vector.x", &lightVec[0], 1.0f, -1.0f);
	// lightVec.y
	ImGui::SliderFloat("Light Vector.y", &lightVec[1], 1.0f, -1.0f);
	// lightVec.y
	ImGui::SliderFloat("Light Vector.z", &lightVec[2], 1.0f, -1.0f);
	for (int i = 0; i < 3; ++i)
	{
		lightVecExp[i] = lightVec[i];
	}

	static float bgCol[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	ImGui::ColorPicker4("BackGround Color", bgCol, ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel |
		ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
	for (int i = 0; i < 4; ++i)
	{
		bgColorExp[i] = bgCol[i];
	}

	static float bloomCol[3] = {};
	ImGui::ColorPicker3("bloom color", bloomCol/*, ImGuiColorEditFlags_::ImGuiColorEditFlags_InputRGB*/);
	for (int i = 0; i < 3; ++i)
	{
		bloomExp[i] = bloomCol[i];
	}

	ImGui::End();

	ImGui::Render();


	D3D12_RESOURCE_BARRIER barrierDesc = CD3DX12_RESOURCE_BARRIER::Transition
	(
		pResoures.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	_cmdList->ResourceBarrier(1, &barrierDesc);

	//auto cmdList = _cmdList;
	auto handle = bufferHeapCreator->GetMultipassRTVHeap()->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * 7;
	const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_cmdList->ClearRenderTargetView(handle, clear_color_with_alpha, 0, nullptr);
	_cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);
	_cmdList->SetDescriptorHeaps(1, imguiSRVHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _cmdList.Get());

	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	_cmdList->ResourceBarrier(1, &barrierDesc);
}

HRESULT SettingImgui::CreateSRVDHeap4Imgui(ComPtr<ID3D12Device> _dev)
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

HRESULT SettingImgui::CreateRTVDHeap4Imgui(ComPtr<ID3D12Device> _dev)
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

HRESULT SettingImgui::CreateBuff4Imgui(ComPtr<ID3D12Device> _dev)
{
	auto dHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resDesc = {};

	constexpr uint32_t shadow_difinition = 1024;
	resDesc.Alignment = 65536;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = 720;
	resDesc.Height = 720;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	

	float clsClr[4] = { 0.5,0.5,0.5,1.0 };
	auto depthClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	auto result = _dev->CreateCommittedResource
	(
		&dHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(imguiBuff.ReleaseAndGetAddressOf())
	);

	return result;
}

void SettingImgui::CreateRTV4Imgui(ComPtr<ID3D12Device> _dev)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvViewDesc = {};
	rtvViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto handle = imguiRTVHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateRenderTargetView
	(
		imguiBuff.Get(),
		&rtvViewDesc,
		handle
	);
}
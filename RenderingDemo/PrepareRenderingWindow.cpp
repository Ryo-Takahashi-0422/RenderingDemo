#include <stdafx.h>
#include "imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM); // @imgui_impl_win32.cpp

LRESULT PrepareRenderingWindow::WndProc(PrepareRenderingWindow* pWindow, HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);
	switch (msg)
	{
	case WM_DESTROY:
		MessageBox(hwnd, TEXT("quit application"),
			TEXT("quit"), MB_ICONINFORMATION);
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		RECT clientRect = {};
		GetClientRect(hWnd, &clientRect);
		OnSizeChanged(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, wp == SIZE_MINIMIZED);

	break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}

// windowがメッセージループ中に取得したメッセージを処理するクラス
LRESULT CALLBACK PrepareRenderingWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{	
	PrepareRenderingWindow* This = (PrepareRenderingWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!This) {//取得できなかった(ウィンドウ生成中)場合
		if (msg == WM_CREATE || msg == WM_INITDIALOG) {
			This = (PrepareRenderingWindow*)((LPCREATESTRUCT)lparam)->lpCreateParams;
			if (This) {				
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)This);
				return This->WndProc(This, hwnd, msg, wparam, lparam);
			}
		}
	}

	else {//取得できた場合(ウィンドウ生成後)
		return This->WndProc(This, hwnd, msg, wparam, lparam);
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void PrepareRenderingWindow::CreateAppWindow(PrepareRenderingWindow* pWindow)
{
	// ウィンドウクラスの生成と初期化
	w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)StaticWndProc;
	w.lpszClassName = _T("DX12Sample");
	w.hInstance = GetModuleHandle(nullptr);

	// 上記ウィンドウクラスの登録。WINDCLASSEXとして扱われる。
	RegisterClassEx(&w);

	RECT wrc = { 0,0, window_width/*1200.0f*/, window_height }; // ★★★★★

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	this->hwnd = CreateWindow(
		w.lpszClassName,
		_T("DX12test"),//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウです
		CW_USEDEFAULT,//表示X座標はOSにお任せします
		CW_USEDEFAULT,//表示Y座標はOSにお任せします
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウハンドル
		nullptr,//メニューハンドル
		w.hInstance,//呼び出しアプリケーションハンドル
		pWindow);//追加パラメータ

	// レンダリングウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
}

void PrepareRenderingWindow::SetViewportAndRect()
{
	viewport = {};
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	scissorRect = {};
	scissorRect.top = 0; //切り抜き上座標
	scissorRect.left = 0; //切り抜き左座標
	scissorRect.right = scissorRect.left + window_width; //切り抜き右座標
	scissorRect.bottom = scissorRect.top + window_height; //切り抜き下座標

	changeableViewport = viewport;
	changeableRect = scissorRect;
}

void PrepareRenderingWindow::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	window_width = width;
	window_height = height;
	if (window_width < MIN_Window_SIZE || window_height < MIN_Window_SIZE)
	{
		winSizeChanged = false;
	}
	else
	{
		winSizeChanged = true;
	}	

	if (winSizeChanged)
	{
		float x = 1.0f;
		float y = 1.0f;

		float viewWidthRatio = 800.0f / window_width;
		float viewHeightRatio = 800.0f / window_height;
		if (viewWidthRatio < viewHeightRatio)
		{
			// The scaled image's height will fit to the viewport's height and 
			// its width will be smaller than the viewport's width.
			x = viewWidthRatio / viewHeightRatio;
		}
		else
		{
			// The scaled image's width will fit to the viewport's width and 
			// its height may be smaller than the viewport's height.
			y = viewHeightRatio / viewWidthRatio;
		}

		D3D12_VIEWPORT m_viewport;
		changeableViewport.Width = window_width * x;
		changeableViewport.Height = window_height * y;
		changeableViewport.TopLeftX = window_width * (1.0f - x) / 2.0f;
		changeableViewport.TopLeftY = window_height * (1.0f - y) / 2.0f;
		changeableViewport.MaxDepth = 1.0f;
		changeableViewport.MinDepth = 0.0f;

		D3D12_RECT m_Rect = {};
		changeableRect.top = static_cast<LONG>(changeableViewport.TopLeftY); //切り抜き上座標
		changeableRect.left = static_cast<LONG>(changeableViewport.TopLeftX); //切り抜き左座標
		changeableRect.right = static_cast<LONG>(changeableViewport.TopLeftX + changeableViewport.Width); //切り抜き右座標
		changeableRect.bottom = static_cast<LONG>(changeableViewport.TopLeftY + changeableViewport.Height); //切り抜き下座標
	}
}

void PrepareRenderingWindow::SetChangeFinished()
{
	winSizeChanged = false;
}
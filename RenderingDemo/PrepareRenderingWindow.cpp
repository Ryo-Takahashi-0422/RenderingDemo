#include <stdafx.h>
#include "imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM); // @imgui_impl_win32.cpp

LRESULT PrepareRenderingWindow::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		MessageBox(hwnd, TEXT("quit application"),
			TEXT("quit"), MB_ICONINFORMATION);
		PostQuitMessage(0);
		return 0;
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
				return This->WndProc(hwnd, msg, wparam, lparam);
			}
		}
	}

	else {//取得できた場合(ウィンドウ生成後)
		return This->WndProc(hwnd, msg, wparam, lparam);
	}

	switch (msg)
	{
	case WM_DESTROY: // process when the window is closed
		MessageBox(hwnd, TEXT("quit the application"),
			TEXT("quit the application"), MB_ICONINFORMATION);
		PostQuitMessage(0);
		return 0;
	// リサイズ処理
	case WM_SIZE:
		if (wparam != SIZE_MINIMIZED)
			This->ResizeWindow();
		break;
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void PrepareRenderingWindow::CreateAppWindow()
{
	// ウィンドウクラスの生成と初期化
	w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)StaticWndProc;
	w.lpszClassName = _T("DX12Sample");
	w.hInstance = GetModuleHandle(nullptr);

	// 上記ウィンドウクラスの登録。WINDCLASSEXとして扱われる。
	RegisterClassEx(&w);

	RECT wrc = { 0,0,window_width, window_height };

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
		nullptr);//追加パラメータ

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

	// 画面サイズ変更トライ
	//float x = 1.0f;
	//float y = 1.0f;

	//float viewWidthRatio = 1024.0f / 1920.0f;
	//float viewHeightRatio = 1024.0f / 1080.0f;
	//x = viewWidthRatio / viewHeightRatio;
	////y = viewHeightRatio / viewWidthRatio;

	//viewport = {};
	//viewport.Width = window_width * x;
	//viewport.Height = window_height * y;
	//viewport.TopLeftX = window_width * (1.0f - x) / 2.0f;
	//viewport.TopLeftY = window_height * (1.0f - y) / 2.0f;
	//viewport.MaxDepth = 1.0f;
	//viewport.MinDepth = 0.0f;

	//scissorRect = {};
	//scissorRect.top = static_cast<LONG>(viewport.TopLeftY); //切り抜き上座標
	//scissorRect.left = static_cast<LONG>(viewport.TopLeftX); //切り抜き左座標
	//scissorRect.right = static_cast<LONG>(viewport.TopLeftX + viewport.Width); //切り抜き右座標
	//scissorRect.bottom = static_cast<LONG>(viewport.TopLeftY + viewport.Height); //切り抜き下座標
	
}

void PrepareRenderingWindow::ResizeWindow()
{
	//RECT clientRect;
	//GetClientRect(hwnd, &clientRect);
	//POINT LT = { clientRect.left, clientRect.top };
	//POINT RB = { clientRect.right, clientRect.bottom };
	//ClientToScreen(hwnd, &LT);
	//ClientToScreen(hwnd, &RB);
}
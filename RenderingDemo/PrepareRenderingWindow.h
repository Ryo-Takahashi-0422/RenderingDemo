#pragma once
#include <windows.h>

class PrepareRenderingWindow
{
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	//! @brief ウィンドウプロシージャ
	virtual LRESULT WndProc(
		HWND hWnd,
		UINT msg,
		WPARAM wp,
		LPARAM lp
	);

	
	WNDCLASSEX w;
	const unsigned int window_width = 720;
	const unsigned int window_height = 720;
	
	HWND hwnd;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

public:
	void CreateAppWindow();
	void SetViewportAndRect();
	HWND GetHWND() { return hwnd; };
	WNDCLASSEX GetWNDCCLASSEX() { return w; };
	D3D12_VIEWPORT GetViewPort() { return viewport; };
	const D3D12_VIEWPORT* GetViewPortPointer() { return &viewport; };
	const D3D12_RECT* GetRectPointer() { return &scissorRect; };
	const unsigned int GetWindowWidth() { return window_width; };
	const unsigned int GetWindowHeight() { return window_height; };
};
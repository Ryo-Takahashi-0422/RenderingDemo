#pragma once
#include <windows.h>
#define MIN_Window_SIZE 800

class PrepareRenderingWindow
{
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	//! @brief ウィンドウプロシージャ
	virtual LRESULT WndProc(
		PrepareRenderingWindow* pWindow,
		HWND hWnd,
		UINT msg,
		WPARAM wp,
		LPARAM lp
	);

	
	WNDCLASSEX w;
	UINT window_width = /*1080*/MIN_Window_SIZE; // 画面サイズとテクスチャサイズが異なる場合、特に太陽の位置がずれることに注意 例：1080に対して太陽テクスチャサイズ1024の場合など
	UINT window_height = /*1080*/MIN_Window_SIZE;
	
	HWND hwnd;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	D3D12_VIEWPORT changeableViewport;
	D3D12_RECT changeableRect;

	//void SetWindowBounds(int left, int top, int right, int bottom);
	void OnSizeChanged(UINT width, UINT height, bool minimized);
	bool winSizeChanged = false;

public:
	void CreateAppWindow(PrepareRenderingWindow* pWindow);
	void SetViewportAndRect();
	HWND GetHWND() { return hwnd; };
	WNDCLASSEX GetWNDCCLASSEX() { return w; };
	D3D12_VIEWPORT GetViewPort() { return viewport; };
	const D3D12_VIEWPORT* GetViewPortPointer() { return &viewport; };
	const D3D12_RECT* GetRectPointer() { return &scissorRect; };

	const D3D12_VIEWPORT* GetChangeableViewPortPointer() { return &changeableViewport; };
	const D3D12_RECT* GetChangeableRectPointer() { return &changeableRect; };

	bool GetWindowSizeChanged() { return winSizeChanged; };
	const unsigned int GetWindowWidth() { return window_width; };
	const unsigned int GetWindowHeight() { return window_height; };
	void SetChangeFinished();
};
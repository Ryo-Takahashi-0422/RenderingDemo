#pragma once
#include <windows.h>
#define MIN_Window_SIZE 1
#define BASE_SIZE 1440

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
	UINT window_width = /*1080*/BASE_SIZE; // 画面サイズとテクスチャサイズが異なる場合、特に太陽の位置がずれることに注意 例：1080に対して太陽テクスチャサイズ1024の場合など
	UINT window_height = /*1080*/BASE_SIZE;
	
	UINT base_width;
	UINT base_height;

	HWND hwnd;

	// シャドウなどのクラスは規定値に基づき解像度を決め、テクスチャを作成する
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	// D3DX12Wrapperで利用する。ウィンドウサイズに変更があった場合にこのクラスで計算した結果を参照させる。
	// swapchainとバックバッファ描画時のviewport, rect設定を可変とする。
	D3D12_VIEWPORT changeableViewport;
	D3D12_RECT changeableRect;

	//void SetWindowBounds(int left, int top, int right, int bottom);
	void OnSizeChanged(UINT width, UINT height, bool minimized);
	bool winSizeChanged = false;

public:
	PrepareRenderingWindow();
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
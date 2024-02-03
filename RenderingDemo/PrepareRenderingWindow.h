#pragma once
#include <windows.h>

class PrepareRenderingWindow
{
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	//! @brief �E�B���h�E�v���V�[�W��
	virtual LRESULT WndProc(
		HWND hWnd,
		UINT msg,
		WPARAM wp,
		LPARAM lp
	);

	
	WNDCLASSEX w;
	const unsigned int window_width = /*1080*/1024; // ��ʃT�C�Y�ƃe�N�X�`���T�C�Y���قȂ�ꍇ�A���ɑ��z�̈ʒu������邱�Ƃɒ��� ��F1080�ɑ΂��đ��z�e�N�X�`���T�C�Y1024�̏ꍇ�Ȃ�
	const unsigned int window_height = /*1080*/1024;
	
	HWND hwnd;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	void ResizeWindow();

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
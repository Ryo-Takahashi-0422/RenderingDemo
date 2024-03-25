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

// window�����b�Z�[�W���[�v���Ɏ擾�������b�Z�[�W����������N���X
LRESULT CALLBACK PrepareRenderingWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{	
	PrepareRenderingWindow* This = (PrepareRenderingWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!This) {//�擾�ł��Ȃ�����(�E�B���h�E������)�ꍇ
		if (msg == WM_CREATE || msg == WM_INITDIALOG) {
			This = (PrepareRenderingWindow*)((LPCREATESTRUCT)lparam)->lpCreateParams;
			if (This) {				
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)This);
				return This->WndProc(This, hwnd, msg, wparam, lparam);
			}
		}
	}

	else {//�擾�ł����ꍇ(�E�B���h�E������)
		return This->WndProc(This, hwnd, msg, wparam, lparam);
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void PrepareRenderingWindow::CreateAppWindow(PrepareRenderingWindow* pWindow)
{
	// �E�B���h�E�N���X�̐����Ə�����
	w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)StaticWndProc;
	w.lpszClassName = _T("DX12Sample");
	w.hInstance = GetModuleHandle(nullptr);

	// ��L�E�B���h�E�N���X�̓o�^�BWINDCLASSEX�Ƃ��Ĉ�����B
	RegisterClassEx(&w);

	RECT wrc = { 0,0, window_width/*1200.0f*/, window_height }; // ����������

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	this->hwnd = CreateWindow(
		w.lpszClassName,
		_T("DX12test"),//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,//�^�C�g���o�[�Ƌ��E��������E�B���h�E�ł�
		CW_USEDEFAULT,//�\��X���W��OS�ɂ��C�����܂�
		CW_USEDEFAULT,//�\��Y���W��OS�ɂ��C�����܂�
		wrc.right - wrc.left,//�E�B���h�E��
		wrc.bottom - wrc.top,//�E�B���h�E��
		nullptr,//�e�E�B���h�E�n���h��
		nullptr,//���j���[�n���h��
		w.hInstance,//�Ăяo���A�v���P�[�V�����n���h��
		pWindow);//�ǉ��p�����[�^

	// �����_�����O�E�B���h�E�\��
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
	scissorRect.top = 0; //�؂蔲������W
	scissorRect.left = 0; //�؂蔲�������W
	scissorRect.right = scissorRect.left + window_width; //�؂蔲���E���W
	scissorRect.bottom = scissorRect.top + window_height; //�؂蔲�������W

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
		changeableRect.top = static_cast<LONG>(changeableViewport.TopLeftY); //�؂蔲������W
		changeableRect.left = static_cast<LONG>(changeableViewport.TopLeftX); //�؂蔲�������W
		changeableRect.right = static_cast<LONG>(changeableViewport.TopLeftX + changeableViewport.Width); //�؂蔲���E���W
		changeableRect.bottom = static_cast<LONG>(changeableViewport.TopLeftY + changeableViewport.Height); //�؂蔲�������W
	}
}

void PrepareRenderingWindow::SetChangeFinished()
{
	winSizeChanged = false;
}
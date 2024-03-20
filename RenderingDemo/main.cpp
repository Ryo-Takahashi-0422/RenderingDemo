#include <stdafx.h>
#include"D3DX12Wrapper.h"
#include<Windows.h>

int main() {
#ifdef _DEBUG
	PIXLoadLatestWinPixGpuCapturerLibrary();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
//#include<Windows.h>
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
//	

	D3DX12Wrapper::D3DX12Wrapper();
	Camera::Camera();
	auto wrapper = D3DX12Wrapper::GetInstance();

	//auto result =wrapper.D3DX12DeviceInit();
	//if (result == E_FAIL) return 0;

	if (!wrapper->PrepareRendering()) {
		return -1;
	}

	if (!wrapper->PipelineInit()) {
		return -1;
	}

	if (!wrapper->ResourceInit()) {
		return -1;
	}

	//wrapper.EffekseerInit();
	
	wrapper->Run();
	//app.Terminate();
	//delete wrapper;
	wrapper->CleanMemory();
	//delete wrapper;
	wrapper->DeleteInstance();
	wrapper = nullptr;
	return 0;
}
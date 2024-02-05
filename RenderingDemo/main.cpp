#include <stdafx.h>
#include"D3DX12Wrapper.h"
#include<Windows.h>
//#ifdef _DEBUG
int main() {
	//PIXLoadLatestWinPixGpuCapturerLibrary();
//#else
//#include<Windows.h>
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
//
//#endif

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
	return 0;
}
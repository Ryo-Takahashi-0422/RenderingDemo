#include <stdafx.h>
#include"D3DX12Wrapper.h"

#ifdef _DEBUG
int main() {
	PIXLoadLatestWinPixGpuCapturerLibrary();
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#endif

	auto& wrapper = D3DX12Wrapper::Instance();

	auto result =wrapper.D3DX12DeviceInit();
	if (result == E_FAIL) return 0;

	if (!wrapper.PrepareRendering()) {
		return -1;
	}
	
	//if (!app.PipelineInit()) {
	//	return -1;
	//}

	//if (!app.ResourceInit()) {
	//	return -1;
	//}

	//app.EffekseerInit();
	
	//app.Run();
	//app.Terminate();
	return 0;
}
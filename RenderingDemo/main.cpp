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
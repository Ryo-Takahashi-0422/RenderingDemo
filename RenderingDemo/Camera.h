#pragma once

class Camera
{
private:
	
	static Camera* instance;
	PrepareRenderingWindow* prepareRenderingWindow = nullptr;

	XMMATRIX world, angle, view, proj;	

public:

	Camera();
	~Camera();
	void Init(PrepareRenderingWindow* _prepareRenderingWindow);
	static Camera* GetInstance() { return instance; };
	XMMATRIX GetWorld() { return world; };
	XMMATRIX GetView() { return view; };
	XMMATRIX GetProj() { return proj; };
};
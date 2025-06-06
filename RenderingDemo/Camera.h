#pragma once

struct Frustum
{
	XMVECTOR topLeft;
	XMVECTOR topRight;
	XMVECTOR bottomLeft;
	XMVECTOR bottomRight;
};

class Camera
{
private:
	
	static Camera* instance;
	PrepareRenderingWindow* prepareRenderingWindow = nullptr;
	Frustum frustum;

	XMMATRIX world, angle, view, proj, invView, invProj;

	XMVECTOR topLeftNear = { -1.0f, 1.0f, 0.0f, 1.0f };
	XMVECTOR topLeftFar = { -1.0f, 1.0f, 0.1f, 1.0f };

	XMVECTOR topRightNear = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMVECTOR topRightFar = { 1.0f, 1.0f, 0.1f, 1.0f };

	XMVECTOR BottomLeftNear = { -1.0f, -1.0f, 0.0f, 1.0f };
	XMVECTOR BottomLeftFar = { -1.0f, -1.0f, 0.1f, 1.0f };

	XMVECTOR BottomRightNear = { 1.0f, -1.0f, 0.0f, 1.0f };
	XMVECTOR BottomRightFar = { 1.0f, -1.0f, 0.1f, 1.0f };

	XMFLOAT3 eye;
	XMFLOAT3 target;

	// for air
	XMFLOAT3 dummyEyePos;
	XMFLOAT3 dummyTargetPos;
	XMMATRIX dummyView;
	Frustum dummyFrustum;

	XMFLOAT3 orbitPos{ 0, 0, 0 };
	XMMATRIX orbitView = XMMatrixIdentity();
	XMMATRIX m_invVP;
public:

	Camera();
	~Camera();
	void Init(PrepareRenderingWindow* _prepareRenderingWindow);
	//void Transform(XMMATRIX transform);
	void MoveCamera(double speed, XMMATRIX charaDirection);
	
	static Camera* GetInstance() { return instance; };
	XMFLOAT3 GetCameraPos() { return eye; };	
	XMMATRIX GetWorld() { return world; };
	XMMATRIX GetView() { return view; };
	XMMATRIX GetInvView() { return m_invVP; };
	XMMATRIX GetProj() { return proj; };
	Frustum GetFrustum() { return frustum; };
	XMMATRIX CalculateOribitView(XMFLOAT3 _charaPos, XMMATRIX _charaDir);

	// air�`��p
	XMFLOAT3 GetDummyCameraPos() { return dummyEyePos; };
	XMFLOAT3 GetDummyTargetPos() { return dummyTargetPos; };
	XMMATRIX GetDummyView() { return dummyView; };
	Frustum GetDummyFrustum() { return dummyFrustum; };
	void SetDummyFrustum();

	void CalculateFrustum();
	XMFLOAT3 GetOrbitCameraPos() { return orbitPos; };
	XMMATRIX GetOrbitView() { return orbitView; };

	void CleanMemory();
};
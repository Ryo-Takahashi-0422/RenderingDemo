#include <stdafx.h>
#include <Camera.h>

Camera* Camera::instance = nullptr;

Camera::Camera()
{
	if (instance == nullptr)
	{
		instance = this;
	}
	world = angle = view = proj = XMMatrixIdentity();
}

Camera::~Camera()
{
	//delete instance;
	delete prepareRenderingWindow;
}

void Camera::Init(PrepareRenderingWindow* _prepareRenderingWindow)
{
	prepareRenderingWindow = _prepareRenderingWindow;
	angle = XMMatrixRotationY(PI);
	//worldMat *= angle; // モデルが後ろ向きなので180°回転して調整

	//ビュー行列の生成・乗算
	XMFLOAT3 eye(0, 1.5, 2.3);
	XMFLOAT3 target(0, 1.5, 0);
	//XMFLOAT3 eye(0, 100, /*0.01*/10);
	//XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	view = XMMatrixLookAtLH
	(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);

	//プロジェクション(射影)行列の生成・乗算
	proj = XMMatrixPerspectiveFovLH
	(
		XM_PIDIV2, // 画角90°
		static_cast<float>(prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(prepareRenderingWindow->GetWindowWidth()),
		1.0, // ニア―クリップ
		3000.0 // ファークリップ
	);
}

void Camera::CalculateFrustum()
{
	invView = XMMatrixInverse(nullptr, view);
	invProj = XMMatrixInverse(nullptr, proj);
	XMFLOAT3 topLeftNear3, topLeftFar3, topRightNear3, topRightFar3, bottomLeftNear3, bottomLeftFar3, bottomRightNear3, bottomRightFar3;

	auto invVP = XMMatrixMultiply(invView, invProj);

	auto topLeftNearTransformed = XMVector4Transform(topLeftNear, invVP);
	topLeftNearTransformed /= topLeftNearTransformed.m128_f32[3];
	auto topLeftFarTransformed = XMVector4Transform(topLeftFar, invVP);
	topLeftFarTransformed /= topLeftFarTransformed.m128_f32[3];

	frustum.topLeft = XMVector4Normalize(XMVectorSubtract(topLeftFarTransformed, topLeftNearTransformed));


	auto topRightNearTransformed = XMVector4Transform(topRightNear, invVP);
	topRightNearTransformed /= topRightNearTransformed.m128_f32[3];
	auto topRightFarTransformed = XMVector4Transform(topRightFar, invVP);
	topRightFarTransformed /= topRightFarTransformed.m128_f32[3];

	frustum.topRight = XMVector4Normalize(XMVectorSubtract(topRightFarTransformed, topRightNearTransformed));


	auto bottomLeftNearTransformed = XMVector4Transform(BottomLeftNear, invVP);
	bottomLeftNearTransformed /= bottomLeftNearTransformed.m128_f32[3];
	auto bottomLeftFarTransformed = XMVector4Transform(BottomLeftFar, invVP);
	bottomLeftFarTransformed /= bottomLeftFarTransformed.m128_f32[3];

	frustum.bottomLeft = XMVector4Normalize(XMVectorSubtract(bottomLeftFarTransformed, bottomLeftNearTransformed));


	auto bottomRightNearTransformed = XMVector4Transform(BottomRightNear, invVP);
	bottomRightNearTransformed /= bottomRightNearTransformed.m128_f32[3];
	auto bottomRightFarTransformed = XMVector4Transform(BottomRightFar, invVP);
	bottomRightFarTransformed /= bottomRightFarTransformed.m128_f32[3];

	frustum.bottomRight = XMVector4Normalize(XMVectorSubtract(bottomRightFarTransformed, bottomRightNearTransformed));
}
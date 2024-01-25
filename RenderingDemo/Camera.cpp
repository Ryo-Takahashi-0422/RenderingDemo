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
	//worldMat *= angle; // ���f�����������Ȃ̂�180����]���Ē���

	//�r���[�s��̐����E��Z
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

	//�v���W�F�N�V����(�ˉe)�s��̐����E��Z
	proj = XMMatrixPerspectiveFovLH
	(
		XM_PIDIV2, // ��p90��
		static_cast<float>(prepareRenderingWindow->GetWindowHeight()) / static_cast<float>(prepareRenderingWindow->GetWindowWidth()),
		1.0, // �j�A�\�N���b�v
		3000.0 // �t�@�[�N���b�v
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
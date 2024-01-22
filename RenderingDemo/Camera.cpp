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

	auto invVP = XMMatrixMultiply(invView, invProj);

	auto topLeftNearTransformed = XMVector4Transform(topLeftNear, invVP);
	auto topLeftFarTransformed = XMVector4Transform(topLeftFar, invVP);
	frustum.topLeft = XMVectorSubtract(topLeftFarTransformed, topLeftNearTransformed);

	auto topRightNearTransformed = XMVector4Transform(topRightNear, invVP);
	auto topRightFarTransformed = XMVector4Transform(topRightFar, invVP);
	frustum.topRight = XMVectorSubtract(topRightFarTransformed, topRightNearTransformed);

	auto bottomLeftNearTransformed = XMVector4Transform(BottomLeftNear, invVP);
	auto bottomLeftFarTransformed = XMVector4Transform(BottomLeftFar, invVP);
	frustum.bottomLeft = XMVectorSubtract(bottomLeftFarTransformed, bottomLeftNearTransformed);

	auto bottomRightNearTransformed = XMVector4Transform(BottomRightNear, invVP);
	auto bottomRightFarTransformed = XMVector4Transform(BottomRightFar, invVP);
	frustum.bottomRight = XMVectorSubtract(bottomRightFarTransformed, bottomRightNearTransformed);
}
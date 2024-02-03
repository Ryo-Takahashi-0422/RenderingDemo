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
	eye = XMFLOAT3(0, 1.5, 2.3);
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
	//invView = XMMatrixInverse(nullptr, view);
	//invProj = XMMatrixInverse(nullptr, proj);

	auto vp = XMMatrixMultiply(view, proj);
	auto invVP = XMMatrixInverse(nullptr, vp);/*XMMatrixMultiply(invView, invProj)*/;
	
	auto topLeftN = XMVector4Transform(topLeftNear, invVP);
	topLeftN /= topLeftN.m128_f32[3];
	auto topLeftF = XMVector4Transform(topLeftFar, invVP);
	topLeftF /= topLeftF.m128_f32[3];

	frustum.topLeft = XMVector4Normalize(XMVectorSubtract(topLeftF, topLeftN));


	auto topRightN = XMVector4Transform(topRightNear, invVP);
	topRightN /= topRightN.m128_f32[3];
	auto topRightF = XMVector4Transform(topRightFar, invVP);
	topRightF /= topRightF.m128_f32[3];

	frustum.topRight = XMVector4Normalize(XMVectorSubtract(topRightF, topRightN));


	auto bottomLeftN = XMVector4Transform(BottomLeftNear, invVP);
	bottomLeftN /= bottomLeftN.m128_f32[3];
	auto bottomLeftF = XMVector4Transform(BottomLeftFar, invVP);
	bottomLeftF /= bottomLeftF.m128_f32[3];

	frustum.bottomLeft = XMVector4Normalize(XMVectorSubtract(bottomLeftF, bottomLeftN));


	auto bottomRightN = XMVector4Transform(BottomRightNear, invVP);
	bottomRightN /= bottomRightN.m128_f32[3];
	auto bottomRightF = XMVector4Transform(BottomRightFar, invVP);
	bottomRightF /= bottomRightF.m128_f32[3];

	frustum.bottomRight = XMVector4Normalize(XMVectorSubtract(bottomRightF, bottomRightN));
}
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
	target = XMFLOAT3(0, 1.5, 0);
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

void Camera::Transform(XMMATRIX transform)
{
	auto tempEye = XMVector4Transform(XMLoadFloat3(&eye), transform);
	//XMStoreFloat3(&eye, tempEye);
	XMFLOAT3 localEyePos;
	XMStoreFloat3(&localEyePos, tempEye);

	auto tempTarget = XMVector4Transform(XMLoadFloat3(&target), transform);
	//XMStoreFloat3(&target, tempTarget);
	XMFLOAT3 localTarget;
	XMStoreFloat3(&localTarget, tempTarget);

	XMFLOAT3 up(0, 1, 0);

	view = XMMatrixLookAtLH
	(
		XMLoadFloat3(&localEyePos),
		XMLoadFloat3(&localTarget),
		XMLoadFloat3(&up)
	);

	CalculateFrustum();
}

void Camera::MoveCamera(double speed, XMMATRIX charaDirection)
{
	// ���s�ړ������ɃL�����N�^�[�̌��������]��������Z���ĕ����ς��B����ɂ���]�ړ������͕s�v�Ȃ̂ŁA1��0�ɂ���BY����]�̂ݑΉ����Ă���B
	auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, -speed), charaDirection);
	moveMatrix.r[0].m128_f32[0] = 1;
	moveMatrix.r[0].m128_f32[2] = 0;
	moveMatrix.r[2].m128_f32[0] = 0;
	moveMatrix.r[2].m128_f32[2] = 1;

	// �ȉ��R�[�h�͑��z�̕`��ɉe������F��������
	//// �J�������W�𓮂���
	//auto tempCameraPos = XMLoadFloat3(&eye);
	//tempCameraPos.m128_f32[3] = 1;

	//tempCameraPos = XMVector4Transform(tempCameraPos, moveMatrix);
	//eye.x = tempCameraPos.m128_f32[0];
	//eye.y = tempCameraPos.m128_f32[1];
	//eye.z = tempCameraPos.m128_f32[2];

	//// �^�[�Q�b�g���W��������
	//auto tempTargetPos = XMLoadFloat3(&target);
	//tempTargetPos.m128_f32[3] = 1;

	//tempTargetPos = XMVector4Transform(tempTargetPos, moveMatrix);
	//target.x = tempTargetPos.m128_f32[0];
	//target.y = tempTargetPos.m128_f32[1];
	//target.z = tempTargetPos.m128_f32[2];

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
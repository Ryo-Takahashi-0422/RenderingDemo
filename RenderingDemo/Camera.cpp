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

void Camera::CleanMemory()
{
	instance = nullptr;
	prepareRenderingWindow = nullptr;
}

void Camera::Init(PrepareRenderingWindow* _prepareRenderingWindow)
{
	prepareRenderingWindow = _prepareRenderingWindow;
	angle = XMMatrixRotationY(PI);
	//worldMat *= angle; // ���f�����������Ȃ̂�180����]���Ē���

	//�r���[�s��̐����E��Z
	eye = XMFLOAT3(0, 1.5, 2.3);	
	target = XMFLOAT3(0, 1.5, 0);
	
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

	// for air
	dummyTargetPos = /*target*/XMFLOAT3(0.0f, 1.5f, -2.3f);
	auto deye = XMFLOAT3(0, 1.5, 0); // eye.z 2,3�̂܂܂���air�̕`�挋�ʂ�z������2.3�I�t�Z�b�g���������ڂɂȂ��Ă��܂�
	dummyEyePos = deye;
	

	auto dtarget = XMFLOAT3(0, 1.5, -1);

	dummyView = XMMatrixLookAtLH
	(
		XMLoadFloat3(&deye),
		XMLoadFloat3(&dtarget),
		XMLoadFloat3(&up)
	);

	
}

//void Camera::Transform(XMMATRIX transform)
//{
//	//auto tempEye = XMVector4Transform(XMLoadFloat3(&dummyEyePos), transform);
//	////XMStoreFloat3(&eye, tempEye);
//	//XMFLOAT3 localEyePos;
//	//XMStoreFloat3(&localEyePos, tempEye);
//
//	//auto tempTarget = XMVector4Transform(XMLoadFloat3(&dummyTargetPos), transform);
//	////XMStoreFloat3(&target, tempTarget);
//	//XMFLOAT3 localTarget;
//	//XMStoreFloat3(&localTarget, tempTarget);
//
//	//XMFLOAT3 up(0, 1, 0);
//
//	//dummyView = XMMatrixLookAtLH
//	//(
//	//	XMLoadFloat3(&localEyePos),
//	//	XMLoadFloat3(&localTarget),
//	//	XMLoadFloat3(&up)
//	//);
//
//	dummyView = XMMatrixMultiply(dummyView, transform);
//
//	auto vp = XMMatrixMultiply(dummyView, proj);
//	auto invVP = XMMatrixInverse(nullptr, vp);/*XMMatrixMultiply(invView, invProj)*/;
//
//	auto topLeftN = XMVector4Transform(topLeftNear, invVP);
//	topLeftN /= topLeftN.m128_f32[3];
//	auto topLeftF = XMVector4Transform(topLeftFar, invVP);
//	topLeftF /= topLeftF.m128_f32[3];
//
//	dummyFrustum.topLeft = XMVector4Normalize(XMVectorSubtract(topLeftF, topLeftN));
//
//
//	auto topRightN = XMVector4Transform(topRightNear, invVP);
//	topRightN /= topRightN.m128_f32[3];
//	auto topRightF = XMVector4Transform(topRightFar, invVP);
//	topRightF /= topRightF.m128_f32[3];
//
//	dummyFrustum.topRight = XMVector4Normalize(XMVectorSubtract(topRightF, topRightN));
//
//
//	auto bottomLeftN = XMVector4Transform(BottomLeftNear, invVP);
//	bottomLeftN /= bottomLeftN.m128_f32[3];
//	auto bottomLeftF = XMVector4Transform(BottomLeftFar, invVP);
//	bottomLeftF /= bottomLeftF.m128_f32[3];
//
//	dummyFrustum.bottomLeft = XMVector4Normalize(XMVectorSubtract(bottomLeftF, bottomLeftN));
//
//
//	auto bottomRightN = XMVector4Transform(BottomRightNear, invVP);
//	bottomRightN /= bottomRightN.m128_f32[3];
//	auto bottomRightF = XMVector4Transform(BottomRightFar, invVP);
//	bottomRightF /= bottomRightF.m128_f32[3];
//
//	dummyFrustum.bottomRight = XMVector4Normalize(XMVectorSubtract(bottomRightF, bottomRightN));
//}

void Camera::MoveCamera(double speed, XMMATRIX charaDirection)
{
	// ���s�ړ������ɃL�����N�^�[�̌��������]��������Z���ĕ����ς��B����ɂ���]�ړ������͕s�v�Ȃ̂ŁA1��0�ɂ���BY����]�̂ݑΉ����Ă���B
	auto moveMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, speed), charaDirection);
	moveMatrix.r[0].m128_f32[0] = 1;
	moveMatrix.r[0].m128_f32[2] = 0;
	moveMatrix.r[2].m128_f32[0] = 0;
	moveMatrix.r[2].m128_f32[2] = 1;
	moveMatrix.r[3].m128_f32[2] *= -1;

	// �ȉ��R�[�h�͑��z�̕`��ɉe������F��������
	// �J�������W�𓮂���
	auto tempCameraPos = XMLoadFloat3(&dummyEyePos);
	tempCameraPos.m128_f32[3] = 1;

	tempCameraPos = XMVector4Transform(tempCameraPos, moveMatrix);
	dummyEyePos.x = tempCameraPos.m128_f32[0];
	dummyEyePos.y = tempCameraPos.m128_f32[1];
	dummyEyePos.z = tempCameraPos.m128_f32[2];

	// �^�[�Q�b�g���W��������
	auto tempTargetPos = XMLoadFloat3(&dummyTargetPos);
	tempTargetPos.m128_f32[3] = 1;

	tempTargetPos = XMVector4Transform(tempTargetPos, moveMatrix);
	dummyTargetPos.x = tempTargetPos.m128_f32[0];
	dummyTargetPos.y = tempTargetPos.m128_f32[1];
	dummyTargetPos.z = tempTargetPos.m128_f32[2];

	XMFLOAT3 up(0, 1, 0);
	view = XMMatrixLookAtLH
	(
		XMLoadFloat3(&dummyEyePos),
		XMLoadFloat3(&dummyTargetPos),
		XMLoadFloat3(&up)
	);

}

void Camera::SetDummyFrustum()
{
	dummyFrustum = frustum;
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

XMMATRIX Camera::CalculateOribitView(XMFLOAT3 _charaPos, XMMATRIX _charaDir)
{
	XMFLOAT3 charaPos = _charaPos;
	XMMATRIX charDir = _charaDir;
	//charaPos.x = charaPos.x;
	//charaPos.y = 1.5f;
	//charaPos.z = charaPos.z;

	XMFLOAT3 up(0, 1, 0);
	XMFLOAT3 z(0.0f, 0.0f, 2.3f);
	XMStoreFloat3(&z, XMVector3Transform(XMLoadFloat3(&z), charDir));

	auto cameraPos = charaPos;
	cameraPos.x += z.x;
	cameraPos.y += z.y;
	cameraPos.z += z.z;
	orbitPos = cameraPos;

	auto oView = XMMatrixLookAtLH
	(
		XMLoadFloat3(&cameraPos),
		XMLoadFloat3(&charaPos),
		XMLoadFloat3(&up)
	);
	dummyEyePos = cameraPos;
	orbitView = oView;

	auto vp = XMMatrixMultiply(orbitView, proj);
	auto invVP = XMMatrixInverse(nullptr, vp);/*XMMatrixMultiply(invView, invProj)*/;
	m_invVP = invVP;
	auto topLeftN = XMVector4Transform(topLeftNear, invVP);
	topLeftN /= topLeftN.m128_f32[3];
	auto topLeftF = XMVector4Transform(topLeftFar, invVP);
	topLeftF /= topLeftF.m128_f32[3];

	dummyFrustum.topLeft = XMVector4Normalize(XMVectorSubtract(topLeftF, topLeftN));


	auto topRightN = XMVector4Transform(topRightNear, invVP);
	topRightN /= topRightN.m128_f32[3];
	auto topRightF = XMVector4Transform(topRightFar, invVP);
	topRightF /= topRightF.m128_f32[3];

	dummyFrustum.topRight = XMVector4Normalize(XMVectorSubtract(topRightF, topRightN));


	auto bottomLeftN = XMVector4Transform(BottomLeftNear, invVP);
	bottomLeftN /= bottomLeftN.m128_f32[3];
	auto bottomLeftF = XMVector4Transform(BottomLeftFar, invVP);
	bottomLeftF /= bottomLeftF.m128_f32[3];

	dummyFrustum.bottomLeft = XMVector4Normalize(XMVectorSubtract(bottomLeftF, bottomLeftN));


	auto bottomRightN = XMVector4Transform(BottomRightNear, invVP);
	bottomRightN /= bottomRightN.m128_f32[3];
	auto bottomRightF = XMVector4Transform(BottomRightFar, invVP);
	bottomRightF /= bottomRightF.m128_f32[3];

	dummyFrustum.bottomRight = XMVector4Normalize(XMVectorSubtract(bottomRightF, bottomRightN));

	return oView;
}
#include <stdafx.h>
#include <Sun.h>

void Sun::Init()
{
	float xRad = 0.0f;
	float yRad = 0.0f;

	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // �r���{�[�h���̂���y�𕉂ɂ���B���z���J�����ւ̃x�N�g��
	direction = pos;
}

XMFLOAT3 Sun::CalculateDirectionFromDegrees(float angleX, float angleY)
{
	
	float xRad = XMConvertToRadians(angleX);
	float yRad = XMConvertToRadians(angleY);
	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // �r���{�[�h���̂���y�𕉂ɂ���B���z���J�����ւ̃x�N�g��
	return pos;
}

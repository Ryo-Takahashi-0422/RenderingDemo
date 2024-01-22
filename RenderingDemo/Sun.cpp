#include <stdafx.h>
#include <Sun.h>

void Sun::Init()
{
	float xRad = 0.0f;
	float yRad = 0.1f;

	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // ビルボード化のためyを負にする。太陽→カメラへのベクトル
	direction = pos;
}
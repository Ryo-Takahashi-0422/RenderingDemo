#include <stdafx.h>
#include <Sun.h>

void Sun::Init()
{
	float xRad = 0.0f;
	float yRad = 0.0f;

	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // ビルボード化のためyを負にする。太陽→カメラへのベクトル
	direction = pos;
	CreateSunVertex();
}

void Sun::CreateSunVertex()
{
	auto div = 2 * PI / (vertexCnt * 2) ;
	auto rad = div;
	XMVECTOR ori = { 0,0,1,1 };
	XMVECTOR begin = { -1,-1,1,1 };
	XMVECTOR end = { 1,1,1,1 };
	XMVECTOR tmpPos;
	for (int i = 0; i < vertexCnt; ++i)
	{
		if (i % 3 == 0)
		{
			vertexes.push_back(ori);
			indices.push_back(i);
			vertexCnt++;
			continue;
		}

		XMVECTOR pos = { cos(rad), sin(rad), 1,1 };

		vertexes.push_back(pos);
		indices.push_back(i);

		rad += div;
	}
}

void Sun::CalculateBillbordMatrix()
{

}

XMFLOAT3 Sun::CalculateDirectionFromDegrees(float angleX, float angleY)
{	
	float xRad = XMConvertToRadians(angleX);
	float yRad = XMConvertToRadians(angleY);
	XMFLOAT3 pos = { cos(yRad) * cos(xRad), -sin(yRad), cos(yRad) * sin(xRad) }; // ビルボード化のためyを負にする。太陽→カメラへのベクトル
	return pos;
}

#pragma once

class Sun
{
private:
	void CreateSunVertex();
	int vertexCnt = 48;
	std::vector<XMVECTOR> vertexes;
	std::vector<int> indices;
	XMFLOAT3 direction;

public:

	void Init();
	void CalculateBillbordMatrix();
	XMFLOAT3 CalculateDirectionFromDegrees(float angleX, float angleY);
	XMFLOAT3 GetDirection() { return direction; };

};

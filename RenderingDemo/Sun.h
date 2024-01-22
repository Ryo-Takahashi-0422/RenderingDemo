#pragma once

class Sun
{
private:

	XMFLOAT3 direction;

public:

	void Init();
	XMFLOAT3 GetDirection() { return direction; };
	
};
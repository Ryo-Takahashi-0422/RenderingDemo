#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
    //return float4(0.0f, 1.0f, 1.0f, 1.0f);
	return float4(input.uv, 1.0f, 1.0f);
}

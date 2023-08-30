#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
 //   float4 result = { 0, 0, 0, 1 };
 //   return result;
	return tex.Sample(smp, input.uv);
	return float4(input.uv, 1.0f, 1.0f);
}

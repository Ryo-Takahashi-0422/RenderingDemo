#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    return float4(input.uv, 1, 1);
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}
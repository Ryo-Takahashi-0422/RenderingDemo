#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    //return input.svpos;
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}
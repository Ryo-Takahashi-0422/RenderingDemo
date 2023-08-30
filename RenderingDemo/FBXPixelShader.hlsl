#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(-light, input.norm.xyz));
    
    return float4(diffuseB, diffuseB, diffuseB, 1);
    //return input.norm;
    return float4(input.uv, 1, 1);
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}
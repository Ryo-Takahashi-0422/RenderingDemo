#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(0, 1, 0));
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(light, input.norm.xyz));
    
    //return colormap.Sample(smp, input.uv);
    //return normalmap.Sample(smp, input.uv);
    //return specularmap.Sample(smp, input.uv);
    //return metalmap.Sample(smp, input.uv);
    //return transparentmap.Sample(smp, input.uv);
    return float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1);// * colormap.Sample(smp, input.uv);
    return input.norm;
    return float4(input.uv, 1, 1);
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}
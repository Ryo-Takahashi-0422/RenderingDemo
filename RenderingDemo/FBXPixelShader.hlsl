#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(0, 1, 0));
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(light, input.norm.xyz));
    
    
    float3 normCol = normalmap.Sample(smp, input.uv);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    
    float bright = dot(input.lightTangentDirection.xyz, normVec);
    bright = max(0, bright);
    bright = saturate(bright);
    
    float4 col = colormap.Sample(smp, input.uv);
    
    //return float4(bright, bright, bright, 1);
    return float4(bright * col.x, bright * col.y, bright * col.z, 1);
    //return normalmap.Sample(smp, input.uv);
    //return specularmap.Sample(smp, input.uv);
    //return metalmap.Sample(smp, input.uv);
    //return transparentmap.Sample(smp, input.uv);
    return /*colormap.Sample(smp, input.uv) * */float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1);
    return input.norm;
    return float4(input.uv, 1, 1);
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}
#include "FBXHeaderShader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(0, 1, 0));
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(light, input.norm.xyz));
    
    
    float3 normCol = normalmap.Sample(smp, input.uv);
    //return float4(normCol, 1);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    //return float4(normVec, 1);
    float3 normal = input.tangent * normVec.x + input.biNormal * normVec.y + input.normal * normVec.z;
    //return float4(normal, 1);
    
    //return float4(input.lightTangentDirection.xyz, 1);
    float bright = dot(input.lightTangentDirection.xyz, normal);
    bright = max(0, bright);
    bright = saturate(bright*2);
    
    float4 col = colormap.Sample(smp, input.uv);
    
    //return col;
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
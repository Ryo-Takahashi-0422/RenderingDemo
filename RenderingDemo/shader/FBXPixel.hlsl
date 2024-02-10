#include "FBXHeader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(0, 1, 0));
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(light, input.norm.xyz));
    float tangentWeight = 1.0f;
    unsigned int biNormalWeight = 0; // 0でUVシームが多少目立たなくなる
    float brightMin = 0.3f;
    float brightEmpha = 2.5f;
    
    // タイリング対応
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
        
    float3 normCol = normalmap.Sample(smp, input.uv);
    //return float4(normCol, 1);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    //return float4(normVec, 1);
    float3 normal = input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
    //return float4(normal, 1);
    
    //return float4(input.lightTangentDirection.xyz, 1);
    float bright = dot(input.lightTangentDirection.xyz, normal);
    bright = max(brightMin, bright);
    bright = saturate(bright * brightEmpha);
    
    float4 col = colormap.Sample(smp, input.uv);
    if (col.a == 0) discard; // アルファ値が0なら透過させる
    
    //return col;
    
    float4 air = airmap.Sample(smp, float3(input.uv, 1));

    float4 renderingResultOfNormalMapAndDiffuseMap = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    //if(col.x !=0)
    //{
    return renderingResultOfNormalMapAndDiffuseMap + air; /* + float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1)*/;
    //}
    //else
    //{
    //    return float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1);
    //}
}
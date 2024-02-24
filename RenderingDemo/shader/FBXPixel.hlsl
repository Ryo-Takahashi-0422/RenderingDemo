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
    
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // 処理対象頂点のスクリーン上の座標。視錐台空間内の座標に対してwで除算して、スクリーンに投影するための立方体の領域（-1≦x≦1、-1≦y≦1そして0≦z≦1）に納める。
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition, eyePos) / 300; // areialの奥行は300
    float4 air = airmap.Sample(smp, float3(scrPos, saturate(airZ)));
    float3 inScatter = air.xyz;

    float4 result = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    //if(col.x !=0)
    //{
    
    
    float4 shadowPos = mul(mul(proj, shadowView), input.worldPosition);
    shadowPos.xyz /= shadowPos.w;
    float2 shadowUV = 0.5 + float2(0.5, -0.5) * /*shadowPos.xy*/(input.lvPos.xy / input.lvPos.w);  
    float2 shadowValue = vsmmap.Sample(smp, shadowUV).xy;

    float4 oriCol = result;
    //shadowPos.z /= 65;
    float lz = input.lvDepth;
    if (input.isChara)
    {
        lz = input.trueDepth;
        shadowValue = float2(vsmmap.Sample(smp, shadowUV).z, vsmmap.Sample(smp, shadowUV).z * vsmmap.Sample(smp, shadowUV).z);
        //shadowValue.x *= 3.0f;
        //shadowValue.y = shadowValue.x * shadowValue.x;

    }
    if (lz /*-0.05f*/ >= shadowValue.x/* && lz <= 1.0f*/)
    {

        float depth_sq = shadowValue.y;
        float var = min(max(depth_sq - shadowValue.y, 0.00005f), 1.0f);
        float md = lz - shadowValue.x;
        float litFactor = var / (var + md * md);

        float3 shadowColor = result.xyz * 0.3f;
        result.xyz = lerp(shadowColor, result.xyz, litFactor);
    }
    
        
    float depth = depthmap.Sample(smp, shadowUV);
    float shadowFactor = 1;
    // こちらを利用する場合はsunのprojを透視投影ビューに切り替えを要する
    //if (shadowPos.z - 0.00001f >= depth) // 時にキャラクターの影に影響している。影の境目が目につく。
    //{
    //    float depth_sq = shadowValue.y;
    //    float var = 0.000000001f;
    //    float md = shadowPos.z - depth;
    //    float litFactor = var / (var + md * md);

    //    float3 shadowColor = result.xyz * 0.3f;
    //    result.xyz = lerp(shadowColor, result.xyz, litFactor);
        
        
    //}
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    result = result * shadowFactor + float4(inScatter, 0);
    
    return result; /* + float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1)*/;
    //}
    //else
    //{
    //    return float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1);
    //}
}
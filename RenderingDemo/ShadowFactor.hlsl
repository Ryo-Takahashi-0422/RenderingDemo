#include "RaySphereIntersection.hlsl"
#include "AtmosphericModel.hlsli"
#include "Definition.hlsli"

#define STEP_CNT 500

RWTexture2D<float4> shadowFactor; // UAVオブジェクト

[numthreads(16, 16, 1)]
void cs_main( uint3 DTid : SV_DispatchThreadID )
{
    // レイの高度(u 0～100km)と、レイ方向を90°とした時に太陽がどの角度にあるか(v -PI/2 ～ PI/2)をベースに、S(x,li) = Vis(li) * T(x,x+tatmoli)の結果をLUTにする
    
    float width, height;
    shadowFactor.GetDimensions(width, height);
    
    float rayHeight = lerp(0, atmosphereRadius - groundRadius, DTid.x / width);
    float2 rayPos = (0, rayHeight + groundRadius);
    float sunTheta = asin(lerp(-1, 1, DTid.y / height)); // レイを90°方向としたときに、太陽がどの角度にあるか

    float2 sunDir = (cos(sunTheta), sin(sunTheta));
    float t = 0;
    
    if (!hasIntersectionWithCircle(rayPos, sunDir, groundRadius)) // Vis(li) 太陽が地表に隠れていない場合
    {
        DiscriminateIntersectionWithCircle(rayPos, sunDir, atmosphereRadius, t);
    }
    else // Vis(li) 太陽が地表に隠れている場合
    {
        shadowFactor[DTid.xy] = float4(0, 0, 0, 1);
        return;
    }
    
    float end = rayPos + sunDir * t; // レイ→太陽方向へ大気圏終端まで
    float3 sumSigmaT = 0;
    
    for (int i = 0; i < STEP_CNT; ++i)
    {
        float cuurentRayPos = lerp(rayPos, end, i / STEP_CNT);
        float h = length(cuurentRayPos) - groundRadius;
        float3 sigmaT = GetSigmaT(h);
        sumSigmaT += sigmaT; // ∫σt(x)
    }
    
    sumSigmaT *= (t / STEP_CNT); // ∫σt(x) * |dx|
    float3 transmittance = exp(-sumSigmaT); // e^-(∫σt(x) * |dx|)

    shadowFactor[DTid.xy] = float4(transmittance, 1); // x:スレッドID xが最小(rayHeight=0)～最大(rayHeight:100) yが最小(sunTheta=-PI/2)～最大(sunTheta:PI/2)の計算結果に対応している。

}
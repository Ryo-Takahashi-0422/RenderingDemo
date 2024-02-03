#include "SunHeader.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    //input.clipPos.xy /= input.clipPos.w;
    
    float4 output = float4(1.0f, 1.0f, 1.0f, 1);
    
    float u, v;
    u = 0.25;
    float sunDir = asin(-sunTheta.y);
    v = (sin(sunDir) + 1) * 0.5;
    float3 sf = shadowFactor.Sample(smp, float2(u, v));
    
    input.position.xyz /= input.position.xyz;
    input.position.xyz *= sf;
    
    return input.position;
}
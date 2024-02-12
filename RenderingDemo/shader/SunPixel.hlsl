#include "SunHeader.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{  
    float u, v;
    u = 0.25;
    float sunDir = asin(-sunTheta.y);
    v = (sin(sunDir) + 1) * 0.5;
    float3 sf = shadowFactor.Sample(smp, float2(u, v));
    sf *= 0.05;
    sf.x += 0.2;
    
    input.position.xyz /= input.position.xyz;
    input.position.xyz *= sf;
    
    return input.position;
}
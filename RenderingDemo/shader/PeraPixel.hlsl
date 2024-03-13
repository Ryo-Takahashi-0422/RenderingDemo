#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{   
    float4 color = tex.Sample(smp, input.uv);
    float4 ssao = ssaoTex.Sample(smp, input.uv);
    
    color = lerp(color * 0.5f, color, ssao.x);
    return color;
}
#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{   
    float4 color = colorTex.Sample(smp, input.uv);
    float4 imgui = imguiTex.Sample(smp, input.uv);
    float4 ssao = ssaoTex.Sample(smp, input.uv);
    color += imgui;
    color = lerp(color * 0.5f, color, ssao.x);
    return color;
}
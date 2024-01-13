#include "PeraHeader.hlsli"

#ifndef POSTCOLOR_HLSL
#define POSTCOLOR_HLSL

#define POSTCOLOR_A 2.51
#define POSTCOLOR_B 0.03
#define POSTCOLOR_C 2.43
#define POSTCOLOR_D 0.59
#define POSTCOLOR_E 0.14

float3 tonemap(float3 input)
{
    // https://tsumikiseisaku.com/blog/shader-tutorial-tonemapping/
    // https://hikita12312.hatenablog.com/entry/2017/08/27/002859
    // ACES Filmic Tonemapping Curve　フィルム調トーンマップ
    return (input * (POSTCOLOR_A * input + POSTCOLOR_B))
         / (input * (POSTCOLOR_C * input + POSTCOLOR_D) + POSTCOLOR_E);
}

float4 ps(Output input) : SV_TARGET
{
    float4 sponza = tex.Sample(smp, input.uv);
    float4 connan = tex2.Sample(smp, input.uv);
    
    float4 cut = connan * 100;
    sponza -= cut;
    sponza = saturate(sponza);
    
    float4 result = sponza + connan;
    float3 col = result;
    col = tonemap(col);    
    col = saturate(pow(col, 1 / 2.2));
    
    return float4(col, 1); 
}

#endif // #ifndef POSTCOLOR_HLSL
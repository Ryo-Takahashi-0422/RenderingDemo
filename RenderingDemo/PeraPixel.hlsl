#include "PeraHeader.hlsli"
#include "Definition.hlsli"

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
    float sponzaDepth = sponzaDepthmap.Sample(smp, input.uv);
    float connanDepth = connanDepthmap.Sample(smp, input.uv);
    
    float4 result;
    if (sponzaDepth < connanDepth)
    {
        result = tex.Sample(smp, input.uv);
    }
    else
    {
        result = tex2.Sample(smp, input.uv);
    }
    
    float3 col = result;
    col = tonemap(col);    
    col = saturate(pow(col, 1 / 2.2));
    
    return float4(col, 1); 
}
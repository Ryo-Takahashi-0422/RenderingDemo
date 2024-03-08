#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{   
    return tex.Sample(smp, input.uv);
}
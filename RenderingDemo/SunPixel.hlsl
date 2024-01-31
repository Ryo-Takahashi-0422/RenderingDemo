#include "SunHeader.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    float4 output = mul(input.position, billboardMatrix);
    
    return output;
}
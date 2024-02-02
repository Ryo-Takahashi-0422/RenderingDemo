#include "SunHeader.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    //input.clipPos.xy /= input.clipPos.w;
    
    float4 output = float4(1.0f, 1.0f, 1.0f, 1);
    
    return input.position;
}
#include "SunHeader.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    float4 output = float4(0.1,0.5,0.5,1);
    
    return output;
}
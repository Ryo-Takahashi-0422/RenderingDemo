#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{
 //   float4 result = { 0, 0, 0, 1 };
 //   return result;
    float4 sponza = tex.Sample(smp, input.uv);
    float4 connan = tex2.Sample(smp, input.uv);
    
    float4 cut = connan * 100;
    sponza -= cut;
    sponza = saturate(sponza);
    
    return sponza + connan; //tex.Sample(smp, input.uv);
	return float4(input.uv, 1.0f, 1.0f);
}

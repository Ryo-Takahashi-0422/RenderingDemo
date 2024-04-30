#include "OcclusionCulling.hlsli"

vsOutput vs_main(float4 pos : POSITION)
{
    vsOutput output;
    output.position = mul(mul(mul(proj, view), world), pos);    
    return output;
}
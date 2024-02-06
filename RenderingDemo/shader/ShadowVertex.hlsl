#include "Shadow.hlsli"

vsOutput vs_main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    vsOutput output;
    output.position = mul(mul(mul(proj, view), world), pos);
    output.texCoord = uv;
    return output;
}
#include "SunHeader.hlsli"

vsOutput vs_main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    vsOutput output;
    output.position = pos;
    output.texCoord = uv;
    return output;
}
#include "SunHeader.hlsli"

vsOutput vs_main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    vsOutput output;
    output.position = mul(float4(pos.x, pos.y, 0, 1), billboardMatrix);
    output.texCoord = uv;
    return output;
}
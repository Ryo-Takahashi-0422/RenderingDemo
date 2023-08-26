#include "BufferHeader.hlsli"

Output vsBuffer(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Output output;
    output.svpos = pos;
    output.uv = uv;
    output.tpos = mul(lightCamera, pos);
    return output;
}

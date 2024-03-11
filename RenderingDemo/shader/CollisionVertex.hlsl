#include "CollisionHeader.hlsli"

Output vs(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Output output;
    pos = mul(rotation, pos);
    output.svpos = mul(mul(mul(proj, view), world), pos);
    output.uv = uv;
    return output;
}

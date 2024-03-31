#include "PeraHeader.hlsli"

Output vs(uint vertexID : SV_VertexID)
{
    Output output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.svpos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0.0, 1);
    return output;
}

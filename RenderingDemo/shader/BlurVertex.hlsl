#include "BlurHeader.hlsli"

vsOutput vs_main(uint vertexID : SV_VertexID)
{
    vsOutput output;
    output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.texCoord * float2(2, -2) + float2(-1, 1), 0.0, 1);
    
    return output;
}
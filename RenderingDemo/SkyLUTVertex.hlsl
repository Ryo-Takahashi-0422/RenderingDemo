#include "SkyLUTHeader.hlsli"
vsOutput vs_main(uint vertexID : SV_VertexID)
{
    // from https://gamedev.stackexchange.com/questions/155183/how-does-the-following-code-generate-a-full-screen-quad
    vsOutput output;
    // output[0].texcoord = float2(0, 0);
    // output[0].position = float4(-1, 1, 0, 1);
    // output[1].texcoord = float2(2, 0);
    // output[1].position = float4(3, 1, 0, 1);
    // output[2].texcoord = float2(0, 2);
    // output[2].position = float4(-1, -3, 0, 1);
    output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.texCoord * float2(2, -2) + float2(-1, 1), 0.5, 1);
 
    return output;
}


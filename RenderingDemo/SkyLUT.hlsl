#include "RaySphereIntersection.hlsl"

cbuffer SceneBuffer : register(b0)
{
    float2 sunDirection; // 
    matrix groundRadius; // view matrix
    matrix atmosphereRadius; // projection matrix
};

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

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

float4 ps_Main(vsOutput input) : SV_TARGET
{
    float phi = input.texCoord.x * PI; // 0~2PI
    float theta = (input.texCoord.y - 1.0f) * 0.5f * PI; // -PI/2 ~ PI/2
    
    float2 dir;
    
    float phaseU = dot(sunDirection, -dir);
    
    return float4(0, 0, 0, 1);

}
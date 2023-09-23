#include "CollisionHeaderShader.hlsli"

float4 ps(Output input) : SV_TARGET
{
    float4 result = { 0.8, 0.8, 0.8, 1 };
    return result;
}

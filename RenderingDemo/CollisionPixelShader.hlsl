#include "CollisionHeaderShader.hlsli"

float4 ps(Output input) : SV_TARGET
{
    float4 result = { 0.0, 0.0, 1.0, 1 };
    return result;
}

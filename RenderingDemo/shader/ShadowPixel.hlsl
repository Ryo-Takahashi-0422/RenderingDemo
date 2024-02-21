#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    return float4(input.depthAndLength.x, input.depthAndLength.y, 0.0f, 0.0f);
}
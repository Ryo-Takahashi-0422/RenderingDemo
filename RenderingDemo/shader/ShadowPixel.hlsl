#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    
    //float depth = length(input.worldPos - lightPos) / 200;
    //float depth2 = depth * depth;
    
    return float4(input.depthAndLength.x/*depth*/, input.depthAndLength.y/*depth2*/, 0.0f, 1.0f);
}
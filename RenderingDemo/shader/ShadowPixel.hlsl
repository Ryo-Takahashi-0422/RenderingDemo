#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    
    //float depth = length(input.worldPos - lightPos) / 100;
    //float depth2 = depth * depth;
    
    return float4(0.0f/*input.depthAndLength.x*//*depth*/, 0.0f/*input.depthAndLength.y*//*depth2*/, 0.0f, 1.0f);
}
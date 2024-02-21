#include "Shadow.hlsli"

vsOutput vs_main
(
    float4 pos : POSITION,
    float4 norm : NORMAL/*_Vertex*/,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive
)
{
    vsOutput output;
    float lAdust = 100.0f;

    if (boneweight1[0] == 0 && boneweight1[1] == 0 && boneweight1[2] == 0 && boneweight2[0] == 0 && boneweight2[1] == 0 && boneweight2[2] == 0)
    {
        //pos -= norm*0.001;
        output.position = mul(mul(proj, view), pos);
        
        float3 worldPos = pos;
        output.depthAndLength.x = length(worldPos - lightPos) / lAdust;
        output.depthAndLength.y = output.depthAndLength.x * output.depthAndLength.x;
        
        output.worldPos = pos;
        
        return output;
    }
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm4 = bones[boneno2[0]] * boneweight2[0];
    matrix bm5 = bones[boneno2[1]] * boneweight2[1];
    matrix bm6 = bones[boneno2[2]] * boneweight2[2];
    
    matrix bm = bm1 + bm2 + bm3 + bm4 + bm5 + bm6;
    
    
    pos = mul(bm, pos);
    pos = mul(rotation, pos);
    //pos -= norm * 0.01;
    output.position = mul(mul(mul(proj, view), world), pos);
    
    float3 worldPos = mul(world, pos);
    output.worldPos = worldPos;
    output.depthAndLength.x = length(worldPos - lightPos) / lAdust;
    output.depthAndLength.y = output.depthAndLength.x * output.depthAndLength.x;
        
    return output;
}
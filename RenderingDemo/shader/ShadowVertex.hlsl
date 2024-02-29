#include "Shadow.hlsli"

vsOutput vs_main
(
    float4 pos : POSITION,
    float4 norm : NORMAL/*_Vertex*/,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
uint index : SV_VertexID
)
{
    vsOutput output;
    output.adjust = 200.0f;

    if (boneweight1[0] == 0 && boneweight1[1] == 0 && boneweight1[2] == 0 && boneweight2[0] == 0 && boneweight2[1] == 0 && boneweight2[2] == 0)
    {
        output.position = mul(mul(proj, view), pos);
        
        // vsm�̏o�͌��ʂ����肳����B���̏������Ȃ���rgb�l�Ƀ������o��B���p���鑤�ł����l�̏������s���K�v����B
        float3 oLightPos = lightPos;
        
        float3 worldPos = pos;
        output.worldPos = pos;
        output.trueDepth = length(worldPos - lightPos) / output.adjust;

        output.depthAndLength.x = length(worldPos - lightPos) / output.adjust;
        output.depthAndLength.y = output.depthAndLength.x * output.depthAndLength.x;
        output.index = index;
        
        output.isChara = false;
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

    output.position = mul(mul(mul(proj, view), world), pos);
    
    float3 oLightPos = lightPos;
    
    float3 worldPos = mul(world, pos);
    output.worldPos = worldPos;
    output.trueDepth = /*0.98f * */length(worldPos - oLightPos) / output.adjust;
    //float jj = worldPos.y + 5.0f;
    //float k = oLightPos.y / jj;
    //oLightPos /= k;
    output.depthAndLength.x = length(worldPos - oLightPos) / output.adjust;
    output.depthAndLength.y = output.depthAndLength.x * output.depthAndLength.x;
    output.isChara = true;
    return output;
}
#include "Shadow.hlsli"

vsOutput vs_main
(
    float4 pos : POSITION,
    float4 norm : NORMAL,
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
    output.specialObj = false;
    if (boneweight1[0] == 0 /*&& boneweight1[1] == 0 && boneweight1[2] == 0 && boneweight2[0] == 0 && boneweight2[1] == 0 */&& boneweight2[2] == 0)
    {
        output.position = mul(mul(proj, view), pos);
        
        // vsm�̏o�͌��ʂ����肳����B���̏������Ȃ���rgb�l�Ƀ������o��B���p���鑤�ł����l�̏������s���K�v����B
        float3 oLightPos = lightPos;
        
        float3 worldPos = pos;
        output.worldPos = pos;
        output.trueDepth = length(worldPos - lightPos) / output.adjust;
        
        // �����y���̂��߃R�����g�A�E�g
        // �|�[���e���L�����N�^�[�w�ʂɊђʂ���̂��ڗ��̂ŁA�΍�B�L�����N�^�[�̃����_�����O���Ƀ|�[���̗����e���ǂ������肷��̂ɗ��p����B
        //if ((77496 <= index && index <= 91841))
        //{
        //    output.specialObj = true;
        //}

        output.index = index;
        
        return output;
    }
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm = bm1 + bm2 + bm3/* + bm4 + bm5 + bm6*/;   
    
    pos = mul(bm, pos);
    pos = mul(rotation, pos);

    output.position = mul(mul(mul(proj, view), world), pos);
    
    float3 oLightPos = lightPos;
    
    float3 worldPos = mul(world, pos);
    output.worldPos = worldPos;
    output.trueDepth = length(worldPos - oLightPos) / output.adjust;

    return output;
}
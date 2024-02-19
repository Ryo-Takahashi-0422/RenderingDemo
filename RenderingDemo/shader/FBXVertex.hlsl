#include "FBXHeader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL_Vertex,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
    float3 tangent : TANGENT,
    float3 binormal : BINORMAL,
    float3 vnormal : NORMAL)
{
    Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm4 = bones[boneno2[0]] * boneweight2[0];
    matrix bm5 = bones[boneno2[1]] * boneweight2[1];
    matrix bm6 = bones[boneno2[2]] * boneweight2[2];
    
    matrix bm = bm1 + bm2 + bm3 + bm4 + bm5 + bm6;
    bool moveObj = true;
    if (boneweight1[0] == 0 && /*boneweight1[1] == 0 && boneweight1[2] == 0 && boneweight2[0] == 0 && boneweight2[1] == 0 &&*/ boneweight2[2] == 0)
    {
        moveObj = false;
        bm[0][0] = 1;
        bm[1][1] = 1;
        bm[2][2] = 1;
        bm[3][3] = 1;
        output.worldPosition = pos;
    }
    else
    {
        pos = mul(bm, pos);
        pos = mul(rotation, pos);
        output.worldPosition = mul(shadowPosMatrix, pos);
    }
    //pos = mul(bm, pos);
    //pos = mul(rotation, pos);

    float4x4 mat;
    mat[0] = float4(tangent, 0.0f);
    mat[1] = float4(binormal, 0.0f);
    mat[2] = float4(norm);
    mat[3] = (0.0f, 0.0f, 0.0f, 1.0f);
    mat = transpose(mat);

    float3 lightDirection = float3(0, 1, 1);
    output.lightTangentDirection = float4(normalize(lightDirection), 1);
     
    float3x3 bmTan;
    bmTan[0] = bm[0];
    bmTan[1] = bm[1];
    bmTan[2] = bm[2];
    output.tangent = normalize(mul(world, mul(bmTan, tangent)));
    output.biNormal = normalize(mul(world, mul(bmTan, binormal)));
    output.normal = normalize(mul(world, mul(bmTan, norm)));
    
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
    //norm.w = 0; // world�ɕ��s�ړ��������܂܂�Ă���ꍇ�A�@�������s�ړ�����B(���̎����f���͈Â��Ȃ�B�Ȃ��H�H)
    output.norm = mul(world, norm);
    //output.vnormal = mul(view, output.norm);
    
    // �^�C�����O�Ή����Ă���uv�ɂ��Ă̏����͔�����B�Ⴆ�Ε��𐳂ɕϊ����鏈�������邱�ƂŁA�e�N�X�`�����΂߂ɂ䂪��
    output.uv = uv;
    
    output.screenPosition = output.svpos;
    //output.worldPosition = pos; //mul(shadowPosMatrix, pos); // world�͒P�ʍs��Ȃ̂ŏ�Z���Ȃ�
    output.worldNormal = normalize(mul(world, norm).xyz);

    return output;
}
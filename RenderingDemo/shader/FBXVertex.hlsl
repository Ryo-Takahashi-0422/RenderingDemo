#include "FBXHeader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL_Vertex,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
    //float3 tangent : TANGENT,
    //float3 binormal : BINORMAL,
    //float3 vnormal : NORMAL,
uint index : SV_VertexID)
{
    Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm4 = bones[boneno2[0]] * boneweight2[0];
    matrix bm5 = bones[boneno2[1]] * boneweight2[1];
    matrix bm6 = bones[boneno2[2]] * boneweight2[2];
    
    matrix bm = bm1 + bm2 + bm3 + bm4 + bm5 + bm6;
    //bool moveObj = true;
    
    output.adjust = 200.0f;
    float3 lightPos;
    float adjustDirValue = 95.0f;
    lightPos.x = adjustDirValue * sunDIr.x;
    lightPos.y = -adjustDirValue * sunDIr.y;
    lightPos.z = adjustDirValue * sunDIr.z;
    output.light = lightPos;
    
    // �ǂ̖@���l��-1�ɂȂ�ssao���v�Z�o�����Ȃ���ւ̑Ώ�
    float4 m_normal = norm;
    float4 oriNorm = norm;
    
    m_normal = normalize(m_normal);
 
    matrix rot = charaRot;
    output.rotatedNorm = m_normal;
    output.rotatedNorm = mul(rot, output.rotatedNorm);
    
    float3 t_normal;
    if (boneweight1[0] == 0 && boneweight2[2] == 0 && sponza)
    {
        m_normal.x *= sign(m_normal.x);
        m_normal.z *= sign(m_normal.z);
        
        //moveObj = false;
        //bm[0][0] = 1;
        //bm[1][1] = 1;
        //bm[2][2] = 1;
        //bm[3][3] = 1;
        output.worldPosition = pos;
        
        output.lvPos = mul(mul(oProj, shadowView), output.worldPosition);
        
        // ���s���e�ɍ��킹�ă��C�g�̈ʒu�𒸓_�̈ʒu�̐^��Ɉړ�������ԂŁA���C�g�����_�ւ̋������Z�o���Ă���B
        //float3 lightPos = -65 * sunDIr;
        //lightPos.x += pos.x/* * lightPos.y / 65.0f*/;
        //lightPos.z += pos.z/* * lightPos.y / 65.0f*/;
        //output.truePos = output.lvPos;
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //float jj = output.worldPosition.y + 5.0f;
        //float k = output.worldPosition.y / jj;
        //lightPos /= k;
        output.isEnhanceShadow = false;
        //if (index <= 25714 || (49897 <= index && index <= 77462) || (77496 <= index && index <= 91841) || (229613 <= index && index <= 233693))
        //{
        //    float newY = output.worldPosition.y + 5.0f;
        //    float div = lightPos.y / newY;
        //    lightPos /= div;
        //    output.isEnhanceShadow = true;
        //}
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65.01f / (lightPos.y + 0.01f);
        output.isChara = false;

        t_normal = normalize(mul(world, m_normal));
    }
    else
    {
        pos = mul(bm, pos);
        pos = mul(rotation, pos);
        output.worldPosition = mul(shadowPosMatrix, pos);
        
        output.lvPos = mul(mul(mul(oProj, shadowView), shadowPosMatrix), pos);
        
        // ���s���e�ɍ��킹�ă��C�g�̈ʒu�𒸓_�̈ʒu�̐^��Ɉړ�������ԂŁA���C�g�����_�ւ̋������Z�o���Ă���B
        //float3 lightPos = -65 * sunDIr;
        //lightPos.x += pos.x/* * lightPos.y / 65.0f*/;
        //lightPos.z += pos.z/* * lightPos.y / 65.0f*/;

        // �L�����N�^�[�̃V���h�E�}�b�v��̕`���2�ʂ肠��B�ʏ�̃��C�g�ʒu����э�����1/n�{�������̂ŁA��҂�sponza�̉e�`��ŗ��p����B�O�҂̓L�����N�^�[�̉e�`��ŗ��p����B
        //output.truePos = output.lvPos;        
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.trueDepth *= 65.01f / (lightPos.y + 0.01f);
        
        float fixY = output.worldPosition.y + 5.0f;
        float enhance = output.worldPosition.y / fixY;
        lightPos /= enhance;
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65 / (lightPos.y + 0.01f);
        output.isEnhanceShadow = true;
        output.isChara = true;
        
        matrix bmTan;
        bmTan[0] = bm[0];
        bmTan[1] = bm[1];
        bmTan[2] = bm[2];
        bmTan[3] = float4(0,0,0,1);
        //bmTan = mul(rot, bmTan);
        bmTan = transpose(bmTan);
        t_normal = normalize(mul(bmTan, output.rotatedNorm));
    }

    float4 m_wPos = mul(world, pos);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = float3(1.0, 0.0, 0.0);
    float dt = dot(t_normal, right);
    float3 m_tangent, m_biTangent;
    if (dt == 0.0f) // -y�������ʂ������Ă���ʂ�normal�Eup�̌��ʂ�nan�ɂȂ邽�ߏ����𕪂���
    {
        m_tangent = normalize(cross(t_normal, normalize(float3(0.1f, 0.8f, 0.1f)))); // �x�N�g���͓K��
        m_biTangent = cross(t_normal, m_tangent);
    }
    else if (abs(abs(dt) - 1.0f) > 0.0f)
    {
        m_tangent = normalize(cross(t_normal, up));
        m_biTangent = cross(t_normal, m_tangent);
    }
    else
    {
        m_tangent = normalize(cross(t_normal, right));
        m_biTangent = cross(t_normal, m_tangent);
    }

    //float3 m_biNormal = cross(t_normal, m_tangent);
    float3 tEye = (float4(eyePos, 1) - m_wPos).xyz;
    float3 tLight = (float4(lightPos, 1) - m_wPos).xyz;
    output.vEyeDirection.x = abs(dot(m_tangent, tEye));
    output.vEyeDirection.y = dot(m_biTangent, tEye);
    output.vEyeDirection.z = abs(dot(t_normal, tEye));
    output.vEyeDirection = normalize(output.vEyeDirection);
    output.vLightDirection.x = dot(m_tangent, tLight);
    output.vLightDirection.y = dot(m_biTangent, tLight);
    output.vLightDirection.z = dot(t_normal, tLight);
    
    // ���z�ʒu���t���ɂȂ��pixelshader�ɂ����鑾�z�����Ɩ@���}�b�v�̓��ς����ɂȂ�A�S�̂��Â��Ȃ錻�ۂ̑΍�B
    // ���z�ʒu��������Ԃł�sponza���Α��̕ǂƃ��C�I���̕`�挋�ʂ��Â��Ȃ邪�A���̏����Ŗ��邢�F�ɓ���o����B
    if (!output.isChara)
    {
        output.vLightDirection.x *= sign(output.vLightDirection.x);
        output.vLightDirection.x *= -1;
        output.vLightDirection.z *= sign(output.vLightDirection.z);
    }

    output.vLightDirection = normalize(output.vLightDirection);
    
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
    //norm.w = 0; // world�ɕ��s�ړ��������܂܂�Ă���ꍇ�A�@�������s�ړ�����B(���̎����f���͈Â��Ȃ�B�Ȃ��H�H)
    //output.norm = mul(world, m_normal);
    


    // �^�C�����O�Ή����Ă���uv�ɂ��Ă̏����͔�����B�Ⴆ�Ε��𐳂ɕϊ����鏈�������邱�ƂŁA�e�N�X�`�����΂߂ɂ䂪��
    output.uv = uv;
    
    output.screenPosition = output.svpos;
    //output.worldPosition = pos; //mul(shadowPosMatrix, pos); // world�͒P�ʍs��Ȃ̂ŏ�Z���Ȃ�
    output.worldNormal = t_normal /*normalize(mul(world, m_normal).xyz)*/ /*normalize(mul(mul(mul(proj, view), world), norm).xyz)*/;
    
    //output.ray = normalize(pos.xyz - eyePos);

    //float3 rotEyepos = mul(rotation, eyePos);
    //float3 rotPos = mul(rotation, pos);
    //output.viewPos = mul(mul(view, world), rotEyepos).xyz - mul(mul(view, world), rotPos).xyz;
    //output.viewPos = mul(-rotation, mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz);
    
    float4 eye = float4(eyePos, 1);
    float4 vEye = mul(view, eye);
    
    output.wPos = m_wPos;
    
    float4 rotPos = mul(rotation, pos);
    float4 vPos = mul(mul(view, world), pos);
    output.viewPos = /*-eye */eye - output.wPos;
    
    output.oriWorldNorm = normalize(mul(world, oriNorm).xyz);
    //output.viewPos.z *= sign(output.viewPos.z);
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    return output;
}
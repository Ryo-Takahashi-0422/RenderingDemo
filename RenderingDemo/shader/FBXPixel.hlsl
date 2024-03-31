#include "FBXHeader.hlsli"

PixelOutput FBXPS(Output input) : SV_TARGET
{
    PixelOutput result;
       
    float4 shadowPos = mul(mul(proj, shadowView), input.worldPosition);
    shadowPos.xyz /= shadowPos.w;
    float2 shadowUV = 0.5 + float2(0.5, -0.5) * /*shadowPos.xy*/(input.lvPos.xy / input.lvPos.w);
    float4 vsmSample = vsmmap.Sample(smp, shadowUV);
    float2 shadowValue = /*vsmmap.Sample(smp, shadowUV)*/vsmSample.xy;

    float lz = input.lvDepth;
    lz = length(input.worldPosition - input.light) / input.adjust;
    
    if (input.isEnhanceShadow)
    {
        lz = input.trueDepth;
        shadowValue = float2(vsmSample.z, vsmSample.z * vsmSample.z);
    }
    
    float tangentWeight = 0.0f;
    float biNormalWeight = 0.0f; // 0��UV�V�[���������ڗ����Ȃ��Ȃ�
    float brightMin = 0.4f;
    float brightEmpha = 4.5f;
    
    float Dot = dot(input.worldNormal, sunDIr);
    float nor = saturate(abs(Dot) + 0.5f);
    
    float3 speclurColor = specularmap.Sample(smp, input.uv).xyz;
    //float3 ray = normalize(input.svpos.xyz - eyePos);
    float3 reflection = normalize(reflect(input.rotatedNorm.xyz, sunDIr));
    float3 speclur = dot(reflection, -input.ray);
    speclur = saturate(speclur);
    speclur *= speclurColor;
    //speclur = pow(speclur, 2);
    speclur *= -sunDIr.y/* * 0.5f*/;
    
    input.normal.x *= charaRot[0].z * sign(sunDIr.x); // ���z��x���W�����ɂ��Z���t�V���h�E�̌����𔽓]�����鏈��
    if (input.isChara)
    {
        brightEmpha = 0.7f;
        nor += 2.1f;
        //brightMin = 0.35f;
        speclur = pow(speclur, 2);
        tangentWeight = 1.0f;
        biNormalWeight = 0.3;
        result.normal = float4(1, 1, 1, 1);
    }
    
    else
    {
        result.normal = float4(input.worldNormal/*input.normal * normVec.z*/, 1);
    }
    // �^�C�����O�Ή�
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
        
    float3 normCol = normalmap.Sample(smp, input.uv);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    
    // ��]�����@����z�O����-����(��������...)�Ȃ̂ŁA���z������z�������}�C�i�X�|������B���̏������Ȃ��Ƒ��z��x����]�ɑ΂���L�����N�^�[�̉A����]�����Ɣ��Α��ɏo�Ă��܂�
    float3 rotatedNorm = normalize(input.rotatedNorm.xyz);
    float3 adjustDir = -sunDIr;
    adjustDir.z *= -1;    
    float rotatedNormDot = dot(rotatedNorm, adjustDir);
    
    float3 normal = rotatedNormDot + input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
      
    // �@���摜�̌��ʂ������_�[�^�[�Q�b�g2�Ɋi�[����
    // �L�����̏ꍇ��SSAO�ŃV���h�E�A�N�����������邽��1�̂݊i�[
    //if (input.isChara)
    //{
    //    result.normal = float4(0, 0, 0, 1);
    //}
    //else
    //{
    
    //}
    
    normal *= -sunDIr.y; // ���z���x���Ⴂ�قǖڗ����Ȃ�����
    // �������L�����N�^�[�ɂ��āA�e�̒��ł͖@���ɂ�郉�C�e�B���O����߂�B�łȂ��Ɖe�̒��ł������������z�����󂯂Ă���悤�Ȍ����ڂɂȂ�B
    if (lz - 0.01f > shadowValue.x && input.isChara)
    {
        normal *= 0.2;
    }

    float bright = dot(abs(input.lightTangentDirection.xyz), normal);
    bright = max(brightMin, bright);
    bright = saturate(bright * brightEmpha);
       
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // �����Ώے��_�̃X�N���[����̍��W�B�������ԓ��̍��W�ɑ΂���w�ŏ��Z���āA�X�N���[���ɓ��e���邽�߂̗����̗̂̈�i-1��x��1�A-1��y��1������0��z��1�j�ɔ[�߂�B
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition, eyePos) / 300; // areial�̉��s��300
    float4 air = airmap.Sample(smp, float3(scrPos, saturate(airZ)));
    float3 inScatter = air.xyz;

    float4 col = colormap.Sample(smp, input.uv);
    if (col.a == 0)
        discard; // �A���t�@�l��0�Ȃ瓧�߂�����
    result.col = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    
    // sponza�ǂ̃|�[�������e���L�����N�^�[���ђʂ���̂��ڗ����ւ̑΍�B�L�����N�^�[�̖@���Ƒ��z�x�N�g���Ƃ̓��ς���L�����N�^�[�w�ʂ��|�[������̗����e���󂯂邩�ǂ����𔻒肷��B
    // �V���h�E�}�b�v���|�[���̒l���ǂ�����vsm�̃A���t�@�l�Ɋi�[����boolean�Ŕ��肵�Ă���B
    if (rotatedNormDot > 0)
    {
        rotatedNormDot = 0;
    }
    rotatedNormDot *= -1;
    bool isSpecial = vsmSample.w;
    // �L�����N�^�[��z���W���͈͈ȏ�A�ȉ��̏ꍇ�����z��x�ʒu�ɂ��L�����N�^�[���|�[������󂯂�V���h�E�}�b�v�̎Q�Ɛ悪�|�[���̏ꍇ�Asponza�̌������ɂ���L�����N�^�[�ɉe�F��薾�邢�т���������
    // ����͌�̃|�[���Q�Ǝ���shadowcolor�𖾂邭���鏈���̌��ʂ�sponza�����ɃL�����N�^�[������Ƃ��̉e�F��薾�邭�Ȃ邩��ŁA�|�[����isSpecia�œ��ʈ������ď����𕪂���݌v�ł̓L�����N�^�[��z���W��
    // ����̈ʒu�Œ��߂��Ă��܂��������Ȃ��B���{�I�ɉe���ђʂ��Ȃ��������Đ݌v����K�v������B
    if (charaPos.z < -6.5f || charaPos.z > 6.4f)
    {
        isSpecial = false;
    }
   
    if (lz /*- 0.01f*/ > shadowValue.x/* && lz <= 1.0f*/)
    {
        float depth_sq = shadowValue.y;
        float var = min(max(depth_sq - shadowValue.y, 0.0001f), 1.0f);
        float md = lz - shadowValue.x;
        float litFactor = var / (var + md * md);              
        float3 shadowColor = result.col.xyz * 0.3f * nor;
        
        // sponza�ǂ̃|�[�������e���L�����N�^�[�̔w�ʂɊђʂ���ꍇ�A�e�F��{���̐F�ɋ߂Â���B�{���̐F��薾�邭�Ȃ�Ȃ��悤��min�Œ������Ă���B
        if (input.isChara && isSpecial)
        {
            shadowColor *= (1.9f + rotatedNormDot * rotatedNormDot) * max(-sunDIr.y, 0.85f);
            shadowColor.x = min(shadowColor.x, result.col.x);
            shadowColor.y = min(shadowColor.y, result.col.y);
            shadowColor.z = min(shadowColor.z, result.col.z);
        }
        result.col.xyz = lerp(shadowColor, result.col.xyz, litFactor);
        speclur *= litFactor;
    }
    
    //return float4(speclur, 1);
        
    float depth = depthmap.Sample(smp, shadowUV);
    float shadowFactor = 1;
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    // �F���������_�[�^�[�Q�b�g1�Ɋi�[����
    result.col = result.col * shadowFactor + float4(inScatter, 0) * airDraw + float4(speclur, 0);
    
    //result.xyz += speclur.xyz;
    
    return result;
}
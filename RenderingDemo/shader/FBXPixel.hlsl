#include "FBXHeader.hlsli"

float4 FBXPS(Output input) : SV_TARGET
{
    float4 result;
       
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
    
    float tangentWeight = 1.0f;
    unsigned int biNormalWeight = 0.3; // 0��UV�V�[���������ڗ����Ȃ��Ȃ�
    float brightMin = 0.3f;
    float brightEmpha = 4.5f;
    
    float Dot = dot(input.worldNormal, sunDIr);
    float nor = saturate(abs(Dot) + 0.5f);
    
    if (input.isChara)
    {
        brightEmpha = 1.3f;
        nor += 0.75f;
        brightMin = 0.2f;
    }
    input.normal.x *= charaRot[0].z * sign(sunDIr.x); // ���z��x���W�����ɂ��Z���t�V���h�E�̌����𔽓]�����鏈��
    
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
    float rotatedNormDot = dot(adjustDir, rotatedNorm);
    
    float3 normal = rotatedNormDot + input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
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
    result = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    
    // sponza�ǂ̃|�[�������e���L�����N�^�[���ђʂ���̂��ڗ����ւ̑΍�B�L�����N�^�[�̖@���Ƒ��z�x�N�g���Ƃ̓��ς���L�����N�^�[�w�ʂ��|�[������̗����e���󂯂邩�ǂ����𔻒肷��B
    // �V���h�E�}�b�v���|�[���̒l���ǂ�����vsm�̃A���t�@�l�Ɋi�[����boolean�Ŕ��肵�Ă���B
    if (rotatedNormDot > 0)
    {
        rotatedNormDot = 0;
    }
    rotatedNormDot *= -1;
    bool isSpecial = vsmSample.w;
    
    float3 speclurColor = specularmap.Sample(smp, input.uv).xyz;
    //float3 ray = normalize(input.svpos.xyz - eyePos);
    float3 reflection = normalize(reflect(sunDIr, input.rotatedNorm.xyz));
    float3 speclur = dot(reflection, -input.ray);
    speclur = saturate(speclur);
    speclur *= speclurColor;
    //speclur = pow(speclur, 2);
    speclur *= -sunDIr.y/* * 0.5f*/;
    
   
    if (lz /*- 0.01f*/ > shadowValue.x/* && lz <= 1.0f*/)
    {
        float depth_sq = shadowValue.y;
        float var = min(max(depth_sq - shadowValue.y, 0.0001f), 1.0f);
        float md = lz - shadowValue.x;
        float litFactor = var / (var + md * md);              
        float3 shadowColor = result.xyz * 0.3f * nor;
        
        // sponza�ǂ̃|�[�������e���L�����N�^�[�̔w�ʂɊђʂ���ꍇ�A�e�F��{���̐F�ɋ߂Â���B�{���̐F��薾�邭�Ȃ�Ȃ��悤��min�Œ������Ă���B
        if (input.isChara && isSpecial)
        {
            shadowColor *= (1.9f + rotatedNormDot * rotatedNormDot) * max(-sunDIr.y, 0.85f);
            shadowColor.x = min(shadowColor.x, result.x);
            shadowColor.y = min(shadowColor.y, result.y);
            shadowColor.z = min(shadowColor.z, result.z);
        }
        result.xyz = lerp(shadowColor, result.xyz, litFactor);
        speclur *= litFactor;
    }
    
    //return float4(speclur, 1);
        
    float depth = depthmap.Sample(smp, shadowUV);
    float shadowFactor = 1;
    //// ������𗘗p����ꍇ��sun��proj�𓧎����e�r���[�ɐ؂�ւ���v����
    //if (shadowPos.z - 0.00001f >= depth) // ���ɃL�����N�^�[�̉e�ɉe�����Ă���B�e�̋��ڂ��ڂɂ��B
    //{
    //    float depth_sq = shadowValue.y;
    //    float var = 0.000000001f;
    //    float md = shadowPos.z - depth;
    //    float litFactor = var / (var + md * md);

    //    float3 shadowColor = result.xyz * 0.3f * nor;
    //    result.xyz = lerp(shadowColor, result.xyz, litFactor);
        
        
    //}
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    result = result * shadowFactor + float4(inScatter, 0) + float4(speclur, 0);
    

    
    //result.xyz += speclur.xyz;
    
    return result;
}
#include "FBXHeader.hlsli"
// lights
bool testLightInRange(const in float lightDistance, const in float cutoffDistance)
{
    return any(float2(cutoffDistance == 0.0, lightDistance < cutoffDistance));
}

float punctualLightIntensityToIrradianceFactor(const in float lightDistance, const in float cutoffDistance/*, const in float decayExponent*/)
{
    //if (decayExponent > 0.0)
    //{
    return /*pow(*/saturate(-lightDistance / cutoffDistance + 1.0)/*, decayExponent)*/;
    //}
  
    //return 1.0;
}

void getDirectionalDirectLightIrradiance(const in DirectionalLight directionalLight, const in GeometricContext geometry, out IncidentLight directLight)
{
    directLight.color = directionalLight.color;
    directLight.direction = directionalLight.direction;
    //directLight.visible = true;
}

void getPointDirectLightIrradiance(const in PointLight pointLight, const in GeometricContext geometry, out IncidentLight directLight)
{
    float3 L = pointLight.position - geometry.position;
    directLight.direction = normalize(L);
  
    float lightDistance = length(L);
    if (testLightInRange(lightDistance, pointLight.distance))
    {
        directLight.color = float3(1, 1, 1);
        directLight.color *= punctualLightIntensityToIrradianceFactor(lightDistance, pointLight.distance/*, pointLight.decay*/);
        //directLight.visible = true;
    }
    else
    {
        directLight.color = float3(0,0,0);
        //directLight.visible = false;
    }
}

// BRDFs
// Normalized Lambert
//float3 DiffuseBRDF(float3 diffuseColor)
//{
//    return diffuseColor / PI;
//}

//float3 F_Schlick(float3 specularColor, float3 H, float3 V)
//{
//    return specularColor + (1.0 - specularColor) * (1.0 - saturate(dot(V, H))) /*pow(1.0 - saturate(dot(V, H)), 5.0))*/;
//}

float D_GGX(float a, float dotNH)
{
    float a2 = a * a;
    float dotNH2 = dotNH * dotNH;
    float d = dotNH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float G_Smith_Schlick_GGX(float a, float dotNV, float dotNL)
{
    float k = a * a * 0.5 + EPSILON;
    float gl = dotNL / (dotNL * (1.0 - k) + k);
    float gv = dotNV / (dotNV * (1.0 - k) + k);
    return gl * gv;
}

// Cook-Torrance
float3 SpecularBRDF(const in IncidentLight directLight, const in GeometricContext geometry, /*float3 specularColor, */float roughnessFactor)
{  
    float3 N = geometry.normal;
    float3 V = geometry.viewDir;
    float3 L = directLight.direction;
  
    float dotNL = saturate(dot(N, L));
    float dotNV = saturate(dot(N, V));
    float3 H = normalize(L + V);

    float dotNH = saturate(dot(N, H));
    //float dotVH = saturate(dot(V, H));
    //float dotLV = saturate(dot(L, V));
    float a = roughnessFactor * roughnessFactor;

    float D = D_GGX(a, dotNH);
    float G = G_Smith_Schlick_GGX(a, dotNV, dotNL);
    //float3 F = F_Schlick(specularColor, V, H);
    return (/*F*/0.5f * (G * D)) / (4.0 * dotNL * dotNV + EPSILON);
}

// RenderEquations(RE)
void RE_Direct(const in IncidentLight directLight, const in GeometricContext geometry, const in Material material, inout ReflectedLight reflectedLight)
{
  
    float dotNL = abs(dot(geometry.normal, directLight.direction));
    float irradiance = dotNL * directLight.color;
  
  // punctual light
    irradiance *= PI;
  
    //reflectedLight.directDiffuse += irradiance * DiffuseBRDF(material.diffuseColor);
    reflectedLight.directSpecular += irradiance * SpecularBRDF(directLight, geometry, /*material.specularColor, */material.specularRoughness);
}

PixelOutput FBXPS(Output input) : SV_TARGET
{    
    float2 dx = ddx(input.uv.x);
    float2 dy = ddy(input.uv.y);
    
    // �^�C�����O�Ή�
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
    
    // svPos.z��depth�l�B �X�N���[���X�y�[�Xuv
    float w, h, d;
    depthmap.GetDimensions(0,w,h,d);
    float2 occuv = float2(input.svpos.x / w, input.svpos.y / h);
    float occDepth = depthmap.Sample(smp, occuv);
    float vPos = input.svpos.z;
    if (vPos - 0.008f >= occDepth)
    {
        discard;
    }
    
    float4 viewSpacePos = input.viewSpacePos;
    float lod = viewSpacePos.z / 8.0f;
    lod = clamp(lod, 0.0f, 6.0f);
    lod = trunc(lod);
    
    float4 col;
    float lodLevel = 3.0f;
    if (lod < lodLevel)
    {
        col = colormap.SampleGrad(smp, input.uv, dx, dy);
    }
    else
    {
        col = colormap.SampleLevel(smp, input.uv, lod);
    }
    
    if (col.a == 0)
        discard; // �A���t�@�l��0�Ȃ瓧�߂�����

    PixelOutput result;
    //float metallic;
    float roughness;
    float3 albedo;
    
    GeometricContext geometry;
    geometry.position = input.wPos;
    geometry.normal = normalize(input.oriWorldNorm);
    geometry.viewDir = normalize(input.viewPos);
  

    
    albedo = col.xyz;
    //metallic = 0.0f;
    float3 speclurColor;
    if (lod < lodLevel)
    {
        speclurColor = specularmap.SampleGrad(smp, input.uv, dx, dy).xyz;
    }
    else
    {
        speclurColor = specularmap.SampleLevel(smp, input.uv, lod).xyz;
    }
    roughness = speclurColor.x;
    
    Material material;
    //material.diffuseColor = lerp(albedo, float3(0,0,0), metallic);
    //material.specularColor = /*lerp(*/float3(0.04f, 0.04f, 0.04f)/*, albedo, metallic)*/;
    material.specularRoughness = roughness;
  
  // Lighting  
    ReflectedLight reflectDirectLight;
    reflectDirectLight.directSpecular = float3(0, 0, 0);
    ReflectedLight reflectPointLight;
    reflectPointLight.directSpecular = float3(0, 0, 0);
    //float3 emissive = float3(0,0,0);
    //float opacity = 1.0;
  
    IncidentLight directLight;
    
    PointLight pointLights[LIGHT_MAX];
    //pointLights[0].color = float3(1, 1, 1);
    pointLights[0].position = /*float3(25.0f, 15.0f, 0.0f)*/plPos1;
    pointLights[0].distance = 100.0f;
    //pointLights[0].decay = 1.0f;
    
    //pointLights[1].color = float3(1, 1, 1);
    pointLights[1].position = /*float3(-25.0f, 15.0f, 0.0f)*/plPos2;
    pointLights[1].distance = 100.0f;
    //pointLights[1].decay = 1.0f;
   
    DirectionalLight directionalLight;
    directionalLight.color = float3(1, 1, 1);
    directionalLight.direction = float3(sunDIr.x, -sunDIr.y, sunDIr.z);
    
    // directional light
    getDirectionalDirectLightIrradiance(directionalLight, geometry, directLight);
    RE_Direct(directLight, geometry, material, reflectDirectLight);
    // point light
    for (int i = 0; i < LIGHT_MAX; ++i)
    {
        getPointDirectLightIrradiance(pointLights[i], geometry, directLight);
        RE_Direct(directLight, geometry, material, reflectPointLight);
    }
    
    reflectPointLight.directSpecular /= 2.0f;
          
    float4 shadowPos = mul(mul(proj, shadowView), input.worldPosition);
    shadowPos.xyz /= shadowPos.w;
    float2 shadowUV = 0.5 + float2(0.5, -0.5) * (input.lvPos.xy / input.lvPos.w);
    float4 vsmSampleGrad = vsmmap. SampleGrad(smp, shadowUV, dx, dy);
    float2 shadowValue = vsmSampleGrad.xy;

    float lz = input.lvDepth;
    lz = length(input.worldPosition - input.light) / input.adjust;
    
    if (input.isEnhanceShadow)
    {
        lz = input.trueDepth;
        shadowValue = float2(vsmSampleGrad.z, vsmSampleGrad.z * vsmSampleGrad.z);
    }
    
    //float3 reflection = normalize(reflect(input.rotatedNorm.xyz, sunDIr));
    //float3 speclur = dot(reflection, -input.ray);
    //speclur = saturate(speclur);
    //speclur *= speclurColor;
    ////speclur = pow(speclur, 2);
    //speclur *= -sunDIr.y;
    float bright = 1.0f;
    result.col = col;
    
    // ��C�̃����_�����O
    float3 dx3 = float3(dx, dx.x);
    float3 dy3 = float3(dy, dy.x);
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // �����Ώے��_�̃X�N���[����̍��W�B�������ԓ��̍��W�ɑ΂���w�ŏ��Z���āA�X�N���[���ɓ��e���邽�߂̗����̗̂̈�i-1��x��1�A-1��y��1������0��z��1�j�ɔ[�߂�B
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition.xyz, eyePos) / 256.0f; // areial�̉��s��256
    float4 air = airmap.SampleGrad(smp, float3(scrPos, airZ), dx3, dy3);
    float3 inScatter = air.xyz;
        
    float3 normCol;
    if (lod < lodLevel)
    {
        normCol = normalmap.SampleGrad(smp, input.uv, dx, dy).xyz;
    }
    else
    {
        normCol = normalmap.SampleLevel(smp, input.uv, lod);
    }
    
    
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    float3 specularNormal = normVec;
    specularNormal.x *= sign(specularNormal.x);
    specularNormal.z *= sign(specularNormal.z);
    specularNormal = normalize(specularNormal);
       
    float diff, spec, spec4;
    float3 eye = normalize(input.vEyeDirection);
    float3 ref;
    //float3 halfLE = normalize(eye);    
    float3 nLightDir = normalize(input.vLightDirection);
    if(input.isChara)
    {
        reflectDirectLight.directSpecular = 0.0f;
        reflectPointLight.directSpecular = 0.0f;

        result.normal = float4(1.0f, 1.0f, 1.0f, 1.0f);
        
        bright = 2.3f;

        normVec.y *= sign(normVec.y);
        normVec = normalize(normVec);
        normVec *= max(0.3f, -sunDIr.y) * 0.8f;

        // �@�����ʌv�Z
        // �������L�����N�^�[�ɂ��āA�e�̒��ł͖@���ɂ�郉�C�e�B���O����߂�B�łȂ��Ɖe�̒��ł������������z�����󂯂Ă���悤�Ȍ����ڂɂȂ�B
        if (lz - 0.01f > shadowValue.x)
        {
            diff = clamp(dot(normVec, nLightDir), 0.3f, 1.0f);
            diff *= 0.5f;
        }
        else
        {
            diff = clamp(dot(normVec, nLightDir), 0.0f, 1.0f);
            diff = pow(diff, 2.0f);
            diff += 0.15f;
            diff = saturate(diff);
        }
        
        // �X�y�L�����[�v�Z
        ref = reflect(-nLightDir, specularNormal);
        ref = normalize(ref);
        spec = dot(eye, ref);
        if (spec < 0)
        {
            spec = 0;
        }
        spec = pow(spec, 40.0f);
        spec4 = float4(spec, spec, spec, 0);
    }
    else
    {
        // �@�����ʌv�Z
        diff = clamp(dot(normVec, nLightDir), 0.0f, 1.0f);
        diff = pow(diff, 2.0f); // �@�����ʋ���
        diff += 0.1f;
        diff = saturate(diff);
        result.normal = float4(normVec, 1);
        
        // �X�y�L�����[�v�Z
        ref = reflect(-nLightDir, specularNormal);
        ref = normalize(ref);
        spec = dot(eye, ref);
        if (spec < 0)
        {
            spec = 0;
        }
        spec = pow(spec, 40.0f);
        spec4 = float4(spec, spec, spec, 0);
    }
    spec4 *= 0.05f;
    result.col *= diff;
        
    // sponza�ǂ̃|�[�������e���L�����N�^�[���ђʂ���̂��ڗ����ւ̑΍�B�L�����N�^�[�̖@���Ƒ��z�x�N�g���Ƃ̓��ς���L�����N�^�[�w�ʂ��|�[������̗����e���󂯂邩�ǂ����𔻒肷��B
    // �V���h�E�}�b�v���|�[���̒l���ǂ�����vsm�̃A���t�@�l�Ɋi�[����boolean�Ŕ��肵�Ă���B
    //if (rotatedNormDot > 0)
    //{
    //    rotatedNormDot = 0;
    //}
    //rotatedNormDot *= -1;
    
    // �����y���̂��߃R�����g�A�E�g
    // �L�����N�^�[��z���W���͈͈ȏ�A�ȉ��̏ꍇ�����z��x�ʒu�ɂ��L�����N�^�[���|�[������󂯂�V���h�E�}�b�v�̎Q�Ɛ悪�|�[���̏ꍇ�Asponza�̌������ɂ���L�����N�^�[�ɉe�F��薾�邢�т���������
    // ����͌�̃|�[���Q�Ǝ���shadowcolor�𖾂邭���鏈���̌��ʂ�sponza�����ɃL�����N�^�[������Ƃ��̉e�F��薾�邭�Ȃ邩��ŁA�|�[����isSpecia�œ��ʈ������ď����𕪂���݌v�ł̓L�����N�^�[��z���W��
    // ����̈ʒu�Œ��߂��Ă��܂��������Ȃ��B���{�I�ɉe���ђʂ��Ȃ��������Đ݌v����K�v������B
    //bool isSpecial = vsmSampleGrad.w;

    //if (charaPos.z < -6.5f || charaPos.z > 6.4f)
    //{
    //    isSpecial = false;
    //}
    

    
    float litFactor;
    if (lz  > shadowValue.x)
    {
        //float depth_sq = shadowValue.y;
        //float var = min(max(depth_sq - shadowValue.y, 0.0001f), 1.0f);
        float var = 0.0001f;
        float md = lz - shadowValue.x;
        litFactor = var / (var + md * md);
        float3 shadowColor = result.col.xyz * 0.4f * bright;
        
        // �����y���̂��߃R�����g�A�E�g
        //// sponza�ǂ̃|�[�������e���L�����N�^�[�̔w�ʂɊђʂ���ꍇ�A�e�F��{���̐F�ɋ߂Â���B�{���̐F��薾�邭�Ȃ�Ȃ��悤��min�Œ������Ă���B
        //if (input.isChara && isSpecial)
        //{
        //    shadowColor *= (4.0f /*+ rotatedNormDot * rotatedNormDot*/) * max(-sunDIr.y, 0.85f);
        //    shadowColor.x = min(shadowColor.x, result.col.x);
        //    shadowColor.y = min(shadowColor.y, result.col.y);
        //    shadowColor.z = min(shadowColor.z, result.col.z);
        //}
        result.col.xyz = lerp(shadowColor, result.col.xyz, litFactor);
        //reflectedLight.directSpecular *= litFactor;
        //speclur *= litFactor;
        //spec4 *= litFactor;
        reflectDirectLight.directSpecular *= litFactor;
    }
    

        
    //float depth = depthmap.SampleGrad(smp, shadowUV);
    float shadowFactor = 1;
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    // �F���������_�[�^�[�Q�b�g1�Ɋi�[����
    float weaken = -sunDIr.y;
    result.col = result.col * shadowFactor + float4(inScatter, 0) * airDraw + /*float4(speclur, 0)*/spec4 * weaken + result.col * float4((reflectDirectLight.directSpecular + reflectPointLight.directSpecular) * brdfDraw, 0) * weaken;
    //result.col = spec4;
    //result.col = float4(diff,diff,diff,0);
    //result.col = /*result.col * */float4((reflectDirectLight.directSpecular + reflectPointLight.directSpecular) * brdfDraw, 0);
    return result;
}
#include "FBXHeader.hlsli"
// lights
bool testLightInRange(const in float lightDistance, const in float cutoffDistance)
{
    return any(float2(cutoffDistance == 0.0, lightDistance < cutoffDistance));
}

void getPointDirectLightIrradiance(const in PointLight pointLight, const in GeometricContext geometry, out IncidentLight directLight)
{
    float3 L = pointLight.position - geometry.position;
    directLight.direction = normalize(L);
  
    float lightDistance = length(L);
    if (testLightInRange(lightDistance, pointLight.distance))
    {
        directLight.color = float3(1, 1, 1);
        directLight.color *= saturate(-lightDistance / pointLight.distance + 1.0);
    }
    else
    {
        directLight.color = float3(0,0,0);
    }
}

float D_GGX(float a2, float dotNH)
{
    //float a2 = a * a;
    float dotNH2 = dotNH * dotNH;
    float d = dotNH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float G_Smith_Schlick_GGX(float a2, float dotNV, float dotNL)
{
    float k = a2 * 0.5 + EPSILON;
    float gl = dotNL / (dotNL * (1.0 - k) + k);
    float gv = dotNV / (dotNV * (1.0 - k) + k);
    return gl * gv;
}

// Cook-Torrance
float3 SpecularBRDF(const in IncidentLight directLight, const in GeometricContext geometry, /*float3 specularColor, */float roughnessFactor, float _dotNL)
{  
    float3 N = geometry.normal;
    float3 V = geometry.viewDir;
    float3 L = directLight.direction;
  
    float dotNL = _dotNL;
    float dotNV = saturate(dot(N, V));
    float3 H = normalize(L + V);

    float dotNH = saturate(dot(N, H));

    float a = roughnessFactor * roughnessFactor;
    float a2 = a * a;

    float D = D_GGX(a2, dotNH);
    float G = G_Smith_Schlick_GGX(a2, dotNV, dotNL);
    return (0.5f * (G * D)) / (4.0 * dotNL * dotNV + EPSILON);
}

// RenderEquations(RE)
void RE_Direct(const in IncidentLight directLight, const in GeometricContext geometry, const in Material material, inout ReflectedLight reflectedLight)
{
  
    float dotNL = abs(dot(geometry.normal, directLight.direction));
    float irradiance = dotNL * directLight.color;
  
  // punctual light
    irradiance *= PI;
    reflectedLight.directSpecular += irradiance * SpecularBRDF(directLight, geometry, material.specularRoughness, dotNL);
}

PixelOutput FBXPS(Output input) : SV_TARGET
{    
    float2 dx = ddx(input.uv.x);
    float2 dy = ddy(input.uv.y);
    
    // タイリング対応
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
    
    // svPos.zはdepth値。 スクリーンスペースuv
    float w, h, d;
    depthmap.GetDimensions(0, w, h, d);
    float2 occuv = float2(input.svpos.x / w, input.svpos.y / h);
    float occDepth = depthmap.Sample(smp, occuv);
    float vPos = input.svpos.z;

    if (vPos - OCC_BIAS >= occDepth)
    {
        discard;
    }
    
    float4 viewSpacePos = input.viewSpacePos;
    float lod = viewSpacePos.z / 8.0f;
    //lod = clamp(lod, 0.0f, 6.0f);
    if(lod > 6.0f)
    {
        lod = 6.0f;
    }
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
        discard; // アルファ値が0なら透過させる

    PixelOutput result;
    float roughness;
    float3 albedo;
    
    GeometricContext geometry;
    geometry.position = input.wPos;
    geometry.normal = input.oriWorldNorm;
    geometry.viewDir = normalize(input.viewPos);
    
    albedo = col.xyz;

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

    material.specularRoughness = roughness;
  
  // Lighting  
    ReflectedLight reflectDirectLight;
    reflectDirectLight.directSpecular = float3(0, 0, 0);
    ReflectedLight reflectPointLight;
    reflectPointLight.directSpecular = float3(0, 0, 0);
  
    IncidentLight directLight;
       
    // directional light
    directLight.color = input.directionalLight.color;
    directLight.direction = input.directionalLight.direction;
    RE_Direct(directLight, geometry, material, reflectDirectLight);
    
    // point light
    for (int i = 0; i < LIGHT_MAX; ++i)
    {
        getPointDirectLightIrradiance(input.pointLights[i], geometry, directLight);
        RE_Direct(directLight, geometry, material, reflectPointLight);
    }
    
    reflectPointLight.directSpecular /= 2.0f;

    float2 shadowUV = 0.5 + float2(0.5, -0.5) * (input.lvPos.xy / input.lvPos.w);
    float4 vsmSampleGrad = vsmmap. SampleGrad(smp, shadowUV, dx, dy);
    float2 shadowValue = vsmSampleGrad.xy;  
    float lz = length(input.worldPosition.xyz - input.light) / input.adjust;
    
    if (input.isEnhanceShadow)
    {
        lz = input.trueDepth;
        shadowValue = float2(vsmSampleGrad.z, vsmSampleGrad.z * vsmSampleGrad.z);
    }
    

    float bright = 1.0f;
    result.col = col;
    
    // 大気のレンダリング
    float3 dx3 = float3(dx, dx.x);
    float3 dy3 = float3(dy, dy.x);
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // 処理対象頂点のスクリーン上の座標。視錐台空間内の座標に対してwで除算して、スクリーンに投影するための立方体の領域（-1≦x≦1、-1≦y≦1そして0≦z≦1）に納める。
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition.xyz, eyePos) / 256.0f; // areialの奥行は256
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
    float3 eye = input.vEyeDirection;
    float3 ref;
    float3 nLightDir = input.vLightDirection;
    if(input.isChara)
    {
        reflectDirectLight.directSpecular = 0.0f;
        reflectPointLight.directSpecular = 0.0f;

        result.normal = float4(1.0f, 1.0f, 1.0f, 1.0f);
        
        bright = 2.3f;

        normVec.y *= sign(normVec.y);
        normVec = normalize(normVec);
        normVec *= max(0.3f, -sunDIr.y) * 0.8f;

        // 法線効果計算
        // 動き回るキャラクターについて、影の中では法線によるライティングを弱める。でないと影の中でもあたかも太陽光を受けているような見た目になる。
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
        
        // スペキュラー計算
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
        // 法線効果計算
        diff = clamp(dot(normVec, nLightDir), 0.0f, 1.0f);
        diff = pow(diff, 2.0f); // 法線効果強調
        diff += 0.1f;
        diff = saturate(diff);
        result.normal = float4(normVec, 1);
        
        // スペキュラー計算
        ref = reflect(-nLightDir, specularNormal);
        ref = normalize(ref);
        spec = dot(eye, ref);
        {
        if (spec < 0)
            spec = 0;
        }
        spec = pow(spec, 40.0f);
        spec4 = float4(spec, spec, spec, 0);
    }
    spec4 *= 0.05f;
    result.col *= diff;
            
    float litFactor;
    if (lz  > shadowValue.x)
    {
        float var = 0.0001f;
        float md = lz - shadowValue.x;
        litFactor = var / (var + md * md);
        float3 shadowColor = result.col.xyz * 0.4f * bright;

        result.col.xyz = lerp(shadowColor, result.col.xyz, litFactor);
        reflectDirectLight.directSpecular *= litFactor;
    }

    float shadowFactor = 1;
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    // 色情報をレンダーターゲット1に格納する
    //float weaken = -sunDIr.y;
    result.col = result.col * shadowFactor + float4(inScatter, 0) * airDraw + spec4 * input.weaken + result.col * float4((reflectDirectLight.directSpecular + reflectPointLight.directSpecular) * brdfDraw, 0) * input.weaken;
    //result.col = spec4;
    return result;
}
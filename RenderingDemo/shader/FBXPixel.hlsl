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

//void getDirectionalDirectLightIrradiance(const in DirectionalLight directionalLight, const in GeometricContext geometry, out IncidentLight directLight)
//{
//    directLight.color = directionalLight.color;
//    directLight.direction = directionalLight.direction;
//    directLight.visible = true;
//}

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
    float dotVH = saturate(dot(V, H));
    float dotLV = saturate(dot(L, V));
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
    // タイリング対応
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
    
    PixelOutput result;
    //float metallic;
    float roughness;
    float3 albedo;
    
    GeometricContext geometry;
    geometry.position = input.wPos;
    geometry.normal = normalize(input.oriWorldNorm);
    geometry.viewDir = normalize(input.viewPos);
  
    float4 col = colormap.Sample(smp, input.uv);
    albedo = col.xyz;
    //metallic = 0.0f;
    float3 speclurColor = specularmap.Sample(smp, input.uv).xyz;
    roughness = speclurColor.x;
    
    Material material;
    //material.diffuseColor = lerp(albedo, float3(0,0,0), metallic);
    //material.specularColor = /*lerp(*/float3(0.04f, 0.04f, 0.04f)/*, albedo, metallic)*/;
    material.specularRoughness = roughness;
  
  // Lighting  
    ReflectedLight reflectedLight;
    //reflectedLight.directDiffuse = float3(0, 0, 0);
    reflectedLight.directSpecular = float3(0, 0, 0);

    //float3 emissive = float3(0,0,0);
    //float opacity = 1.0;
  
    IncidentLight directLight;
    
    PointLight pointLights[LIGHT_MAX];
    //pointLights[0].color = float3(1, 1, 1);
    pointLights[0].position = float3(25.0f, 15.0f, /*4.5*/0.0f);
    pointLights[0].distance = 100.0f;
    //pointLights[0].decay = 1.0f;
    
    //pointLights[1].color = float3(1, 1, 1);
    pointLights[1].position = float3(-25.0f, 15.0f, /*4.5*/0.0f);
    pointLights[1].distance = 100.0f;
    //pointLights[1].decay = 1.0f;
   
  // point light
    for (int i = 0; i < LIGHT_MAX; ++i)
    {
        if (i >= LIGHT_MAX)
            break;
        getPointDirectLightIrradiance(pointLights[i], geometry, directLight);
        RE_Direct(directLight, geometry, material, reflectedLight);
    }
  
       
    float4 shadowPos = mul(mul(proj, shadowView), input.worldPosition);
    shadowPos.xyz /= shadowPos.w;
    float2 shadowUV = 0.5 + float2(0.5, -0.5) * (input.lvPos.xy / input.lvPos.w);
    float4 vsmSample = vsmmap.Sample(smp, shadowUV);
    float2 shadowValue = vsmSample.xy;

    float lz = input.lvDepth;
    lz = length(input.worldPosition - input.light) / input.adjust;
    
    if (input.isEnhanceShadow)
    {
        lz = input.trueDepth;
        shadowValue = float2(vsmSample.z, vsmSample.z * vsmSample.z);
    }
    
    float tangentWeight = 0.0f;
    float biNormalWeight = 0.0f; // 0でUVシームが多少目立たなくなる
    float brightMin = 0.4f;
    float brightEmpha = 4.5f;
    
    float Dot = dot(input.worldNormal, sunDIr);
    float nor = saturate(abs(Dot) + 0.5f);
    
    float3 reflection = normalize(reflect(input.rotatedNorm.xyz, sunDIr));
    float3 speclur = dot(reflection, -input.ray);
    speclur = saturate(speclur);
    speclur *= speclurColor;
    //speclur = pow(speclur, 2);
    speclur *= -sunDIr.y;
    
    input.normal.x *= charaRot[0].z * sign(sunDIr.x); // 太陽のx座標符号によりセルフシャドウの向きを反転させる処理
    if (input.isChara)
    {
        brightEmpha = 0.7f;
        nor += 2.1f;
        //brightMin = 0.35f;
        speclur = float3(0, 0, 0);
        tangentWeight = 1.0f;
        biNormalWeight = 0.3;
        result.normal = float4(1, 1, 1, 1);
        reflectedLight.directSpecular = 0.0f;
    }
    else
    {
        result.normal = float4(input.worldNormal, 1);
    }

        
    float3 normCol = normalmap.Sample(smp, input.uv);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    
    // 回転した法線のz前方は-方向(直したい...)なので、太陽方向もz成分をマイナス掛けする。この処理がないと太陽のx軸回転に対するキャラクターの陰が回転方向と反対側に出てしまう
    float3 rotatedNorm = normalize(input.rotatedNorm.xyz);
    float3 adjustDir = -sunDIr;
    adjustDir.z *= -1;
    float rotatedNormDot = dot(rotatedNorm, adjustDir);
    
    float3 normal = rotatedNormDot + input.tangent * tangentWeight * normVec + input.biNormal * normVec * biNormalWeight + input.normal * normVec;
       
    normal *= -sunDIr.y; // 太陽高度が低いほど目立たなくする
    // 動き回るキャラクターについて、影の中では法線によるライティングを弱める。でないと影の中でもあたかも太陽光を受けているような見た目になる。
    if (lz - 0.01f > shadowValue.x && input.isChara)
    {
        normal *= 0.2;
    }

    float bright = dot(abs(input.lightTangentDirection.xyz), normal);
    bright = max(brightMin, bright);
    bright = saturate(bright * brightEmpha);
       
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // 処理対象頂点のスクリーン上の座標。視錐台空間内の座標に対してwで除算して、スクリーンに投影するための立方体の領域（-1≦x≦1、-1≦y≦1そして0≦z≦1）に納める。
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition, eyePos) / 300; // areialの奥行は300
    float4 air = airmap.Sample(smp, float3(scrPos, saturate(airZ)));
    float3 inScatter = air.xyz;

    //float4 col = colormap.Sample(smp, input.uv);
    if (col.a == 0)
        discard; // アルファ値が0なら透過させる
    result.col = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    
    // sponza壁のポール落ち影がキャラクターを貫通するのが目立つ問題への対策。キャラクターの法線と太陽ベクトルとの内積からキャラクター背面がポールからの落ち影を受けるかどうかを判定する。
    // シャドウマップがポールの値かどうかはvsmのアルファ値に格納したbooleanで判定している。
    if (rotatedNormDot > 0)
    {
        rotatedNormDot = 0;
    }
    rotatedNormDot *= -1;
    bool isSpecial = vsmSample.w;
    // キャラクターのz座標が範囲以上、以下の場合かつ太陽のx位置によりキャラクターがポールから受けるシャドウマップの参照先がポールの場合、sponzaの建物内にいるキャラクターに影色より明るい帯が発生する
    // これは後のポール参照時のshadowcolorを明るくする処理の結果がsponza屋内にキャラクターがいるときの影色より明るくなるからで、ポールをisSpeciaで特別扱いして処理を分ける設計ではキャラクターのz座標を
    // 特定の位置で調節してごまかすしかない。根本的に影が貫通しない処理を再設計する必要がある。
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
        
        // sponza壁のポール落ち影がキャラクターの背面に貫通する場合、影色を本来の色に近づける。本来の色より明るくならないようにminで調整している。
        if (input.isChara && isSpecial)
        {
            shadowColor *= (1.9f + rotatedNormDot * rotatedNormDot) * max(-sunDIr.y, 0.85f);
            shadowColor.x = min(shadowColor.x, result.col.x);
            shadowColor.y = min(shadowColor.y, result.col.y);
            shadowColor.z = min(shadowColor.z, result.col.z);
        }
        result.col.xyz = lerp(shadowColor, result.col.xyz, litFactor);
        //reflectedLight.directSpecular *= litFactor;
        speclur *= litFactor;
    }
        
    float depth = depthmap.Sample(smp, shadowUV);
    float shadowFactor = 1;
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    // 色情報をレンダーターゲット1に格納する
    result.col = result.col * shadowFactor + float4(inScatter, 0) * airDraw + float4(speclur, 0) + result.col * float4(/*reflectedLight.directDiffuse * 0.05f + */reflectedLight.directSpecular, 0) * -sunDIr.y;
    
    return result;
}
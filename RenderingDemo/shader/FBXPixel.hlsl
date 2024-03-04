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
    unsigned int biNormalWeight = 0.3; // 0でUVシームが多少目立たなくなる
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
    input.normal.x *= charaRot[0].z * sign(sunDIr.x); // 太陽のx座標符号によりセルフシャドウの向きを反転させる処理
    
    // タイリング対応
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
        
    float3 normCol = normalmap.Sample(smp, input.uv);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    
    // 回転した法線のz前方は-方向(直したい...)なので、太陽方向もz成分をマイナス掛けする。この処理がないと太陽のx軸回転に対するキャラクターの陰が回転方向と反対側に出てしまう
    float3 rotatedNorm = normalize(input.rotatedNorm.xyz);
    float3 adjustDir = -sunDIr;
    adjustDir.z *= -1;    
    float rotatedNormDot = dot(adjustDir, rotatedNorm);
    
    float3 normal = rotatedNormDot + input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
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

    float4 col = colormap.Sample(smp, input.uv);
    if (col.a == 0)
        discard; // アルファ値が0なら透過させる
    result = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    
    // sponza壁のポール落ち影がキャラクターを貫通するのが目立つ問題への対策。キャラクターの法線と太陽ベクトルとの内積からキャラクター背面がポールからの落ち影を受けるかどうかを判定する。
    // シャドウマップがポールの値かどうかはvsmのアルファ値に格納したbooleanで判定している。
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
        
        // sponza壁のポール落ち影がキャラクターの背面に貫通する場合、影色を本来の色に近づける。本来の色より明るくならないようにminで調整している。
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
    //// こちらを利用する場合はsunのprojを透視投影ビューに切り替えを要する
    //if (shadowPos.z - 0.00001f >= depth) // 時にキャラクターの影に影響している。影の境目が目につく。
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
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
    float biNormalWeight = 0.0f; // 0でUVシームが多少目立たなくなる
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
    
    input.normal.x *= charaRot[0].z * sign(sunDIr.x); // 太陽のx座標符号によりセルフシャドウの向きを反転させる処理
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
    float rotatedNormDot = dot(rotatedNorm, adjustDir);
    
    float3 normal = rotatedNormDot + input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
      
    // 法線画像の結果をレンダーターゲット2に格納する
    // キャラの場合はSSAOでシャドウアクメが発生するため1のみ格納
    //if (input.isChara)
    //{
    //    result.normal = float4(0, 0, 0, 1);
    //}
    //else
    //{
    
    //}
    
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
        speclur *= litFactor;
    }
    
    //return float4(speclur, 1);
        
    float depth = depthmap.Sample(smp, shadowUV);
    float shadowFactor = 1;
    
    if (-sunDIr.y <= 0.7f)
    {
        shadowFactor = max(0.3f, (-sunDIr.y + 0.3f));
    }
    
    // 色情報をレンダーターゲット1に格納する
    result.col = result.col * shadowFactor + float4(inScatter, 0) * airDraw + float4(speclur, 0);
    
    //result.xyz += speclur.xyz;
    
    return result;
}
#include "RaySphereIntersection.hlsli"
#include "AtmosphericModel.hlsli"

cbuffer Frustum : register(b1) // 現在カメラが描画しているスクリーン座標4隅のワールド空間におけるベクトル
{
    float4 topLeftFrustum;
    float4 topRightFrustum;
    float4 bottomLeftFrustum;
    float4 bottomRightFrustum;
};

cbuffer SkyLUTBuffer : register(b2)
{
    matrix world;
    matrix sunViewMatrix;
    matrix sunProjMatrix;
    float3 eyePos;
    float depthLength;
    float3 adjustedEyePos;
    float distanceLimit;
    float3 sunDirection;
};

RWTexture3D<float4> AirTexture; // 出力先UAVオブジェクト
SamplerState smp : register(s0); // No.0 sampler
Texture2D<float> shadowMap : register(t0);
Texture2D<float3> shadowFactor : register(t1);

[numthreads(16, 16, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
    // カメラからレイを各ピクセル方向へ飛ばし、奥行をzとして3Dテクスチャにシングルスキャッタリングの値を累加して格納していく。スキャッタリングの計算をするかはレイの位置におけるz値を太陽から見た
    
    float width, height, depth;
    AirTexture.GetDimensions(width, height, depth);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float3 currentPixelDir =
    normalize(
        lerp(lerp(topLeftFrustum, topRightFrustum, (DTid.x + 0.5) / width),
             lerp(bottomLeftFrustum, bottomRightFrustum, (DTid.x + 0.5) / width), (DTid.y + 0.5) / height));
    //currentPixelDir.x *= -1;
    
    float startT = 0;
    float divDepth = /*depthLength / depth*/0.7f;
    //float endT = min(divDepth, distanceLimit);
    float maxT;
    float3 cameraPos3D = float3(0, adjustedEyePos.y + groundRadius, 0);
    if (!DiscriminateIntersectionWithSphere(cameraPos3D, currentPixelDir, groundRadius, maxT))
    {
        DiscriminateIntersectionWithSphere(cameraPos3D, currentPixelDir, atmosphereRadius, maxT);
    }
    float endT = min(divDepth, maxT);
    
    // 乱数生成器　https://nimushiki.com/2018/10/09/1252/#fracsindotcoxy_float212989878233_437585453
    float rand = frac(sin(dot(
        float2(DTid.x + 0.5f, DTid.y + 0.5f), float2(12.9898, 78.233) * 2.0)) * 43758.5453);
    
    float3 scattering = float3(0, 0, 0);
    float3 sumSigmaT = float3(0, 0, 0);    
    
    // レイマーチング
    for (int z = 0; z < depth; ++z)
    {
        float dt = (endT - startT);
        float nextT = startT + dt;

        float randT = lerp(startT, nextT, rand);
        float3 rayPos = cameraPos3D + randT * currentPixelDir; // レイ位置を現在処理中のピクセル方向へrandT分進める
        
        // 現在のレイが太陽から見て影の中にあるか判定する
        bool isShadow = true;
        if (!hasIntersectionWithSphere(rayPos, -sunDirection, groundRadius))
        {
            //float4 shadowPos = mul(mul(mul(sunProjMatrix, sunViewMatrix), world), float4(rayPos, 1)); // 現在のレイ位置を、太陽から見たカメラの視錐台座標に変換
            float3 shadowPos = eyePos + randT * currentPixelDir/* / 10.0f*/; // カメラからピクセル方向へレイを進める
            float4 framedShadowPos = mul(mul(mul(sunProjMatrix, sunViewMatrix), world), float4(shadowPos, 1));
            framedShadowPos.xyz /= framedShadowPos.w; // xyをwで割って、それら値を太陽から見たカメラの視錐台空間(-1～+1)に収めている
            float2 shadowUV = 0.5 + float2(0.5, -0.5) * framedShadowPos.xy; // xはそのまま0～1に調整し、yは空間の上端が0に、下端が1になるように調整。yの下端が1なのはv下端が1で、カメラから見た絵に変換された状態のレイ位置をuvで表す

            float shadowZ = shadowMap.SampleLevel(smp, shadowUV, 0);
            float rayZ = framedShadowPos.z;
            isShadow = rayZ > shadowZ - 0.0001f; // レイのz位置が太陽から見たデプスマップより同位置で取得したz値より大きければ、レイ位置は遮蔽されて影である
        }
        
        // 現在のレイが太陽から見て影でないならスキャッタリングの計算を行う
        if (!isShadow)
        {
            float h = length(rayPos) - groundRadius;
            float3 sigmaS = GetSigmaS(h);
            float3 sigmaT = GetSigmaT(h);
            float3 deltaSigmaT = dt * sigmaT; // T(c,x)
            float3 transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));
            float phaseTheta = dot(sunDirection, -currentPixelDir);
            float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
            float angleBetweenSunlightAndRay = PI / 2 - acos(dot(-sunDirection, normalize(rayPos)));
        
            float u, v;
            u = h / (atmosphereRadius - groundRadius);
            v = (sin(angleBetweenSunlightAndRay) + 1) * 0.5;
            float3 sf = shadowFactor.SampleLevel(smp, float2(u, v), 0); // S(x,li)計算
        
            scattering += dt * sigmaS * transmittanceFromRayToEye * phaseFuncResult * sf;
            sumSigmaT += deltaSigmaT;
        }
        
        //float temp = scattering.x;
        //scattering.x = scattering.z;
        //scattering.z = temp;
        //temp = scattering.y;
        //scattering.y = scattering.z;
        //scattering.z = temp;
        //scattering.x *= 1.02;
        
        AirTexture[int3(DTid.xy, z)] = float4(scattering * 10000, 1);
        startT = nextT;
        endT = min(endT + divDepth, distanceLimit);
    }
    
    
}
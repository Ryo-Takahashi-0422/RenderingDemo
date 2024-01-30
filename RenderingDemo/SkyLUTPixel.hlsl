#include "SkyLUTHeader.hlsli"

void rayMarching(inout float3 scattering, inout float3 sumSigmaT, float currentT, float nextT, float phaseTheta, float3 dir, float3 cameraPos)
{
    float3 rayPos = cameraPos + nextT * dir;
    float h = length(rayPos) - groundRadius;

    float3 sigmaS = GetSigmaS(h);
    float3 sigmaT = GetSigmaT(h);
    float3 deltaSigmaT = sigmaT * (nextT - currentT); // T(c,x)
    float3 transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));

    if (!hasIntersectionWithSphere(rayPos, -sunDirection, groundRadius))
    {
        float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
        float angleBetweenSunlightAndRay = PI / 2 - acos(dot(-sunDirection, normalize(rayPos)));
        
        float u, v;
        u = h / (atmosphereRadius - groundRadius);
        v = (sin(angleBetweenSunlightAndRay) + 1) * 0.5;
        float3 sf = shadowFactor.Sample(smp, float2(u, v));
        // S(x,li)計算
        
        scattering += (nextT - currentT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult * sf;
    }

    sumSigmaT += deltaSigmaT;
}

float4 ps_main(vsOutput input) : SV_TARGET
{
    float x = input.position.x / (width - 1);
    float y = input.position.y / (height - 1);
    // origin
    //float phi = input.texCoord.x * 2 * PI; // 0~2PI
    //float theta = (input.texCoord.y - 1.0f) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 
    //float phi = input.texCoord.x * 2 * PI; // 0~2PI
    //float theta = (2 * input.texCoord.y - 0.5) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 

    float phi = x * 2 * PI; // 0~2PI
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float theta = (y - 0.5f)/* * 0.5f*/ * PI; // -PI/2 ~ PI/2 ; // PI / 2 を掛けてy軸の範囲を-PI/2～PI/2に変化させている。 sign：x < 0 ? -1: 1　vm*vmで線形→非線形化している。vm二乗による符号の消失をsign(vm)で残している。
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    
    float2 twoDimDir = float2(cosTheta, sinTheta); // 視線方向をxy平面で定義する。空は一様であるため球面座標におけるphi(x→z)は考慮しない。
    float2 cameraPos2D = float2(0, eyePos.y + groundRadius);
    float endT;
    if (!DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, groundRadius, endT))
    {
        DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, atmosphereRadius, endT);
    }

    // 球面座標でレイマーチング
    float3 cameraPos3D = float3(0, eyePos.y + groundRadius, 0);
    float3 dir = float3(cosTheta * cosPhi, sinTheta, cosTheta * sinPhi); // スクリーン座標を球面座標に変換する。レイ方向は解像度 / 360°確保出来る。
    float3 scattering = float3(0, 0, 0);
    float3 sumSigmaT = float3(0, 0, 0);
    
    float deltaT = endT / stepCnt;
    float currentT = 0;
    float nextT = 0;
    float phaseTheta = dot(sunDirection, -dir);
    
    for (int i = 0; i < stepCnt; ++i)
    {
        nextT += deltaT;
        rayMarching(scattering, sumSigmaT, currentT, nextT, phaseTheta, dir, cameraPos3D);
        currentT = nextT;
    }
    
    return float4(scattering * sunIntensity * 10, 1);

}
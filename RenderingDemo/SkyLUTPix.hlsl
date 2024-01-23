//#include "RaySphereIntersection.hlsl"
//#include "AtmosphericModel.hlsl"
//#include "Definition.hlsli"
//struct vsOutput
//{
//    float4 position : SV_POSITION;
//    float2 texCoord : TEXCOORD;
//};
//cbuffer SkyKUTBuffer : register(b1)
//{
//    float3 eyePos;
//    float3 sunDirection; // 
//    float stepCnt;
//    float3 sunIntensity;
//};
#include "SkyLUTHeader.hlsli"
void rayMarching(inout float3 scattering, inout float3 sumSigmaT, float currentT, float nextT, float phaseTheta, float3 dir, float3 cameraPos)
{
    float3 pos = cameraPos + nextT * dir;
    float h = length(pos) - groundRadius;

    float sigmaS = GetSigmaS(h);
    float sigmaT = GetSigmaT(h);
    float deltaSigmaT = sigmaT * (nextT - currentT); // T(c,x)
    float transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));

    if (hasIntersectionWithSphere(pos, sunDirection, atmosphereRadius))
    {
        float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
        float angleBetweenSunlightAndRay = dot(sunDirection, -dir);
        // S(x,li)計算
        
        scattering += (nextT - currentT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult /* * S(x,li) */;
    }

    sumSigmaT += deltaSigmaT;
}
float4 ps_main(vsOutput input) : SV_TARGET
{
    float phi = input.texCoord.x * PI; // 0~2PI
    float theta = (input.texCoord.y - 1.0f) * 0.5f * PI; // -PI/2 ~ PI/2 
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float2 twoDimDir = (cosTheta, sinTheta); // 視線方向をxy平面で定義する。空は一様であるため球面座標におけるphi(x→z)は考慮しない。
    float2 cameraPos2D = (0, eyePos.y + groundRadius);
    float endT;
    if (!DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, groundRadius, endT))
    {
        DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, atmosphereRadius, endT);
    }
    //return float4(0.0f,1.0f,0.0f,1.0f);
    // 球面座標でレイマーチング
    float3 cameraPos3D = (0, eyePos.y + groundRadius, 0);
    float3 dir = (cosTheta * cosPhi, sinTheta, cosTheta * sinPhi); // スクリーン座標を球面座標に変換する。レイ方向は解像度 / 360°確保出来る。
    float3 scattering = (0, 0, 0);
    float3 sumSigmaT = (0, 0, 0);
    
    float deltaT = endT / stepCnt;
    float currentT = 0;
    float nextT = 0;
    float phaseTheta = dot(sunDirection, -dir);
    
    for (int i = 0; i < stepCnt; ++i)
    {
        nextT += deltaT;
        rayMarching(scattering, sumSigmaT, currentT, nextT, phaseTheta, dir, cameraPos3D);
        currentT += deltaT;
    }
    
    //
    
    return float4(scattering * sunIntensity, 1);

}
#include "RaySphereIntersection.hlsl"
#include "AtmosphericModel.hlsl"
#include "Defines.hlsli"

cbuffer SkyKUTBuffer : register(b1)
{
    float3 eyePos;
    float3 sunDirection; // 
    float stepCnt;
    float3 sunIntensity;
};

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

vsOutput vs_main(uint vertexID : SV_VertexID)
{
    // from https://gamedev.stackexchange.com/questions/155183/how-does-the-following-code-generate-a-full-screen-quad
    vsOutput output;
    // output[0].texcoord = float2(0, 0);
    // output[0].position = float4(-1, 1, 0, 1);
    // output[1].texcoord = float2(2, 0);
    // output[1].position = float4(3, 1, 0, 1);
    // output[2].texcoord = float2(0, 2);
    // output[2].position = float4(-1, -3, 0, 1);
    output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.texCoord * float2(2, -2) + float2(-1, 1), 0.5, 1);
    return output;
}

void rayMarching(out float3 scattering, out float3 sumSigmaT, float currentT, float nextT, float phaseTheta, float3 dir, float3 cameraPos)
{
    float3 pos = cameraPos + nextT * dir;
    float h = length(pos) - groundRadius;

    float sigmaS = GetSigmaS(h);
    float sigmaT = GetSigmaT(h);
    float transmittanceFromRayToEye = sigmaT * (nextT - currentT); // T(c,x)

    if (hasIntersectionWithSphere(pos, sunDirection, atmosphereRadius))
    {
        float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
        // p(v,li)計算
        // S(x,li)計算
        
        scattering += (nextT - currentT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult /* * S(x,li) */;
    }

    sumSigmaT += sigmaT;
}

float4 ps_Main(vsOutput input) : SV_TARGET
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
    
    // 球面座標でレイーマチングする。
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


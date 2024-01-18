#include "RaySphereIntersection.hlsl"
#include "AtmosphericModel.hlsl"

cbuffer SceneBuffer : register(b0)
{
    float3 eyePos;
    float2 sunDirection; // 
    float stepCnt;
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

void rayMarching(out float3 scattering, out float3 sumSigmaS, float sumT, float deltaT, float phaseTheta, float2 dir, float2 origin)
{
    float3 pos = origin + deltaT * dir;
    float h = length(pos) - groundRadius;

    float sigmaS = GetSigmaS(h, sigmaS);
    float transmittanceFromRayToEye = sigmaS * (sumT + deltaT);

    
}

float4 ps_Main(vsOutput input) : SV_TARGET
{
    float phi = input.texCoord.x * PI; // 0~2PI
    float theta = (input.texCoord.y - 1.0f) * 0.5f * PI; // -PI/2 ~ PI/2 
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float2 dir = (cosTheta, sinTheta); // 視線方向をxy平面で定義する。空は一様であるため球面座標におけるphi(x→z)は考慮しない。
    
    //float3 dir = (cosTheta * cosPhi, sinTheta, cosTheta * sinPhi); // スクリーン座標を球面座標に変換する。
    
    float2 origin = (0, eyePos.y + groundRadius);
    
    float endT;
    if (!DiscriminateIntersectionWithCircle(origin, dir, groundRadius, endT))
    {
        DiscriminateIntersectionWithCircle(origin, dir, atmosphereRadius, endT);
    }
    
    float3 scattering = (0, 0, 0);
    float3 sumSigmaS = (0, 0, 0);
    float sumT = 0;
    float deltaT = endT / stepCnt;
    float phaseTheta = dot(sunDirection, -dir);
    
    for (int i = 0; i < stepCnt; ++i)
    {
        rayMarching(scattering, sumSigmaS, sumT, deltaT, phaseTheta, dir, origin);
        sumT += deltaT;
    }
    
    //
    
    return float4(scattering, 1);

}


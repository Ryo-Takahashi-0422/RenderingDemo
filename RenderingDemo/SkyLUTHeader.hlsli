#include "RaySphereIntersection.hlsl"
#include "AtmosphericModel.hlsli"
#include "Definition.hlsli"

cbuffer SkyLUTBuffer : register(b1)
{
    float3 eyePos;
    float3 sunDirection; // 
    float stepCnt;
    float3 sunIntensity;
};

SamplerState smp : register(s0); // No.0 sampler
Texture2D<float3> shadowFactor : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    //float3 _eyePos : EYEPOS;
    //float3 _sunDirection : SUNDIRECTION;
    //float _stepCnt : STEPCNT;
    //float3 _sunIntensity : SUNINTENSITY;
};
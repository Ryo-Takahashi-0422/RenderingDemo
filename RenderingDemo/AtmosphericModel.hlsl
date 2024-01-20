#include "Definition.hlsli"

cbuffer AtmosBuffer : register(b0)
{
    float3 rayleighScattering;
    float mieScattering;
    float mieAbsorption;
    float3 ozoneAbsorption;
    float asymmetryParameter;
    float altitudeOfRayleigh;
    float altitudeOfMie;
    float halfWidthOfOzone;
    float altitudeOfOzone;

    float groundRadius;
    float atmosphereRadius;
};

float3 GetSigmaS(float h)
{
    float3 rayleighSigmaS = rayleighScattering * exp(-h / altitudeOfRayleigh);
    float mieSigmaS = mieScattering * exp(-h / altitudeOfMie);

    float3 sigmaS = rayleighSigmaS + mieSigmaS;
    return sigmaS;
}

float3 GetSigmaT(float h)
{
    float3 rayleighSigmaT = rayleighScattering * exp(-h / altitudeOfRayleigh);
    float mieSigmaT = (mieScattering + mieAbsorption) * exp(-h / altitudeOfMie);
    
    float ozoneDistribution = max(0.0f, 1.0f - abs(h - altitudeOfOzone) / halfWidthOfOzone);
    float ozoneSigmaT = ozoneAbsorption * ozoneDistribution;
    
    float3 sigmaT = rayleighSigmaT + mieSigmaT + ozoneSigmaT;
    return sigmaT;
}

float CalculatePhaseFunctiuon(float theta)
{
    float phaseRayleigh = 3 * (1 + cos(theta) * cos(theta));
    
    float g = asymmetryParameter;
    float m = 1 + g * g - 2 * g * cos(theta);
    float phaseMie = (1 - g * g) / (4 * PI * m * sqrt(m));

    return phaseRayleigh + phaseMie;

}
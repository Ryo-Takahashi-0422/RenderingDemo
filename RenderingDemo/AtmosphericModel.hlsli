#include "Definition.hlsli"

cbuffer ParticipatingMedia : register(b0)
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
    float3 ozoneSigmaT = ozoneAbsorption * ozoneDistribution;
    
    float3 sigmaT = rayleighSigmaT + mieSigmaT + ozoneSigmaT;
    return sigmaT;
}

float CalculatePhaseFunctiuon(float theta)
{
    float theta2 = theta * theta;
    float phaseRayleigh = 3 * (1 + theta2) / (16 * PI);
    
    float g = asymmetryParameter;
    float g2 = g * g;
    float m = 1 + g2 - 2 * g * theta;
    float phaseMie = 3 / (8 * PI) * (1 - g2) * (1 + theta2) / ((2 + g2) * m * sqrt(m)); // Cornette-Shanks
    //float phaseMie = (1 - g * g) / (4 * PI * m * sqrt(m)); 別式：ヘニエイグリーンスタイン

    return phaseRayleigh + phaseMie;

}
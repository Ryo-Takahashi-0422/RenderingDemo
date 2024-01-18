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

void GetSigmaS(float h, out float sigmaS)
{
    float3 rayleighSigmaS = rayleighScattering * exp(-h / altitudeOfRayleigh);
    float mieSigmaS = mieScattering * exp(-h / altitudeOfMie);

    sigmaS = rayleighSigmaS + mieSigmaS;

}

void GetSigmaT(float h, out float sigmaT)
{
    float3 rayleighSigmaT = rayleighScattering * exp(-h / altitudeOfRayleigh);
    float mieSigmaT = (mieScattering + mieAbsorption) * exp(-h / altitudeOfMie);
    
    float ozoneDistribution = max(0.0f, 1.0f - abs(h - altitudeOfOzone) / halfWidthOfOzone);
    float ozoneSigmaT = ozoneAbsorption * ozoneDistribution;
    
    sigmaT = rayleighSigmaT + mieSigmaT + ozoneSigmaT;

}
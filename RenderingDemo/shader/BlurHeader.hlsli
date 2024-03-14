struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer GaussianWeight : register(b0) // gaussian weight
{
    float4 gaussianWeights[2];
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0); // No.0 sampler
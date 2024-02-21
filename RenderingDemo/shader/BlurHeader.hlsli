struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D<float4> shadowRendering : register(t0);
SamplerState smp : register(s0); // No.0 sampler
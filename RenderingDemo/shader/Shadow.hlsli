cbuffer WVPMatrix : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};


SamplerState smp : register(s0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};
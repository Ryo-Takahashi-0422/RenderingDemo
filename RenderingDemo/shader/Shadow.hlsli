cbuffer WVPMatrix : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix rotation;
    matrix bones[256];
};


SamplerState smp : register(s0);

struct vsOutput
{
    float4 position : SV_POSITION;
};
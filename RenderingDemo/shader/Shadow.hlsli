cbuffer WVPMatrix : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix rotation;
    matrix bones[256];
    float3 lightPos;
};


SamplerState smp : register(s0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    float trueDepth : TRUE_DEPTH;
    float adjust : ADJUST;
    float index : INDEX;
    bool specialObj : SPECIALOBJ;
};
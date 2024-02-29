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
    float2 depthAndLength : DEPTH_LENGTH;
    float trueDepth : TRUE_DEPTH;
    bool isChara : IS_CHARACTER;
    float adjust : ADJUST;
    float index : INDEX;
};
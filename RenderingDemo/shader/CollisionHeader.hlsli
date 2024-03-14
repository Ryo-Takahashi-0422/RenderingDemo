struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer SceneBuffer : register(b0) // ïœä∑çsóÒ
{
    matrix rotation;
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    bool isDraw;
    //matrix bones[256]; // pmd bone matrix // index number is equal with bones index number
};

SamplerState smp : register(s0); // No.0 sampler
Texture2D<float4> colorTex : register(t0);
Texture2D<float4> imguiTex : register(t1);
Texture2D<float4> ssaoTex : register(t2);
Texture2D<float4> bluredColor : register(t3);
Texture2D<float4> depthMap : register(t4);
//Texture2D<float4> imguiWindow : register(t5); // skyLUT
//Texture2D<float4> sunTex : register(t6); // skyLUT

SamplerState smp : register(s0); // ƒTƒ“ƒvƒ‰[

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0); // sponza描画
Texture2D<float4> tex2 : register(t1); // キャラクター描画
Texture2D<float4> sponzaDepthmap : register(t2); // sponzaデプスマップ
Texture2D<float4> connanDepthmap : register(t3); // キャラクターデプスマップ
Texture2D<float4> sky : register(t4); // skyLUT
Texture2D<float4> imguiWindow : register(t5); // skyLUT

SamplerState smp : register(s0); // サンプラー

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

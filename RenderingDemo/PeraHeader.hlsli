Texture2D<float4> tex : register(t0); // sponza描画
Texture2D<float4> tex2 : register(t1); // キャラクター描画
SamplerState smp : register(s0); // サンプラー

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

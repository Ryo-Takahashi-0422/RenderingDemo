cbuffer Frustum : register(b0) // 現在カメラが描画しているスクリーン座標4隅のワールド空間におけるベクトル
{
    float4 topLeft;
    float4 topRight;
    float4 bottomLeft;
    float4 bottomRight;
};

SamplerState smp : register(s0);
Texture2D<float4> SkyLUT : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};
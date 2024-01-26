cbuffer Frustum : register(b0) // 現在カメラが描画しているスクリーン座標4隅のワールド空間におけるベクトル
{
    float4 topLeftFrustum;
    float4 topRightFrustum;
    float4 bottomLeftFrustum;
    float4 bottomRightFrustum;
};

cbuffer worldMatrix : register(b1) // キー押下によるworld座標変化
{
    matrix world;
};

SamplerState smp : register(s0);
Texture2D<float4> SkyLUT : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 topLeft : TOPLEFT;
    float3 topRight : TOPRIGHT;
    float3 bottomLeft : BOTTOMLEFT;
    float3 bottomRight : BOTTOMRIGHT;
};
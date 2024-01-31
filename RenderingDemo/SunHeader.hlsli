cbuffer BillboardMatrix : register(b0) // 現在カメラが描画しているスクリーン座標4隅のワールド空間におけるベクトル
{
    matrix billboardMatrix;
};

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};
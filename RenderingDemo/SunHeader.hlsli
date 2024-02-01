cbuffer BillboardMatrix : register(b0) // 現在カメラが描画しているスクリーン座標4隅のワールド空間におけるベクトル
{
    matrix world;
    matrix view;
    matrix proj;
    matrix cameraPos;
    matrix sunDir;
    matrix billboard;
};

struct vsOutput
{
    float4 position : SV_POSITION;
    float4 clipPos : CLIP_POSITION;
    float2 texCoord : TEXCOORD;
};
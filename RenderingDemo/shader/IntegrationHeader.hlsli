struct vsOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 color : SV_TARGET0; // integrated color
    float4 normal : SV_TARGET1; // integrated normal
    float4 imgui : SV_TARGET2; // integrated depth
};

SamplerState smp : register(s0); // サンプラー

Texture2D<float4> tex : register(t0); // color thread1
Texture2D<float4> tex2 : register(t1); // color thread2
Texture2D<float4> sponzaDepthmap : register(t2); // sponzaデプスマップ
Texture2D<float4> connanDepthmap : register(t3); // キャラクターデプスマップ
Texture2D<float4> sky : register(t4); // skyLUT
Texture2D<float4> imguiWindow : register(t5); // imgui
Texture2D<float4> sunTex : register(t6); // sun
Texture2D<float4> normal1 : register(t7); // normal thread1
Texture2D<float4> normal2 : register(t8); // normal thread2
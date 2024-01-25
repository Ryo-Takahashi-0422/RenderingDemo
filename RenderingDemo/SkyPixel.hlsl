#include "SkyHeader.hlsli"
#include "Definition.hlsli"

#define POSTCOLOR_A 2.51
#define POSTCOLOR_B 0.03
#define POSTCOLOR_C 2.43
#define POSTCOLOR_D 0.59
#define POSTCOLOR_E 0.14

float3 tonemap(float3 input)
{
    // https://tsumikiseisaku.com/blog/shader-tutorial-tonemapping/
    // https://hikita12312.hatenablog.com/entry/2017/08/27/002859
    // ACES Filmic Tonemapping Curve　フィルム調トーンマップ
    return (input * (POSTCOLOR_A * input + POSTCOLOR_B))
         / (input * (POSTCOLOR_C * input + POSTCOLOR_D) + POSTCOLOR_E);
}
float4 ps_main(vsOutput input) : SV_TARGET
{
	// ワールド空間における、カメラ→処理するピクセルへのベクトルを求める。
    float3 topLeft = topLeftFrustum.xyz;
    float3 topRight = topRightFrustum.xyz;
    float3 bottomLeft = bottomLeftFrustum.xyz;
    float3 bottomRight = bottomRightFrustum.xyz;    
    
    float3 currentPixelVector = normalize(lerp(lerp(topLeft, topRight, input.texCoord.x), lerp(bottomLeft, bottomRight, input.texCoord.x), input.texCoord.y));
    float theta = asin(currentPixelVector.y);
	
    float3 phi = atan2(currentPixelVector.x, currentPixelVector.z);
    
    float u, v;
    u = (phi / (2 * PI));
    v = (sin(theta) + 1) * 0.5;
    
    float4 col = SkyLUT.Sample(smp, float2(u, v));
    col.xyz = tonemap(col.xyz);
    
    return col;

}
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
    //float3 topLeft = topLeftFrustum.xyz;
    //float3 topRight = topRightFrustum.xyz;
    //float3 bottomLeft = bottomLeftFrustum.xyz;
    //float3 bottomRight = bottomRightFrustum.xyz;
    
    float3 topLeft = input.topLeft;
    float3 topRight = input.topRight;
    float3 bottomLeft = input.bottomLeft;
    float3 bottomRight = input.bottomRight;
      
    //float x = input.texCoord.x;
    //float y = input.texCoord.y;
    float x = input.position.x / 255;
    float y = input.position.y / 255;
    float3 currentPixelDir = 
    normalize(
        lerp(lerp(topLeft, topRight, x),
             lerp(bottomLeft, bottomRight, x), y));
    
    float u, v;
    float theta = asin(currentPixelDir.y);	
    float phi = clamp(atan2(currentPixelDir.z, currentPixelDir.x) + 2 * PI, 0, 2 * PI);    
    v = 0.5 + 0.5 * sign(theta) * sqrt(abs(theta) / (PI / 2));
    u = (phi / (PI) + 1);

    
    float4 sky = SkyLUT.Sample(smp, float2(u, v));
    sky.xyz = tonemap(sky.xyz);
    
    return sky;

}
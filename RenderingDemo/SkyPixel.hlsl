#include "SkyHeader.hlsli"
#include "Definition.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
	// ワールド空間における、カメラ→処理するピクセルへのベクトルを求める。
    float4 currentPixelVector = lerp(lerp(topLeft, topRight, input.texCoord.x), lerp(bottomLeft, bottomRight, input.texCoord.x), input.texCoord.y);
    float eyeAngle = currentPixelVector.y;
	
    float3 normX = (1, 0, 0);
    float3 currentPixelNormalX = (currentPixelVector.x, 0, currentPixelVector.z);
    float phi = acos(dot(normX, currentPixelNormalX));
    
    
    float u, v;
    u = phi;
    v = (eyeAngle + 1) * 0.5;
    
    return SkyLUT.Sample(smp, float2(u, v));
}
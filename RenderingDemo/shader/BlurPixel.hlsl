#include "BlurHeader.hlsli"

float4 SimpleGaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv /*, float dx, float dy*/)
{
    float4 ret = float4(0, 0, 0, 0);
    
    float w, h, levels;
    _texture.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    // highest
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 2 * dy)) * 1;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 2 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 2 * dy)) * 1;
    // high
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 1 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 1 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 1 * dy)) * 4;
    // middle
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 0 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 0 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 0 * dy)) * 36;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 0 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 0 * dy)) * 6;
    // low
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, -1 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, -1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, -1 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, -1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, -1 * dy)) * 4;
    // lowest
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, -2 * dy)) * 1;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, -2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, -2 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, -2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, -2 * dy)) * 1;
 
    return ret / 256;
}

float4 ps_main(vsOutput input) : SV_TARGET
{
    float4 blur = /*shadowRendering.Sample(smp, input.texCoord)*/SimpleGaussianBlur(shadowRendering, smp, input.texCoord);
    return blur;
}
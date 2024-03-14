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

float4 Get5x5GaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv, float dx, float dy)
{
    float4 ret = float4(0, 0, 0, 0);
    ret = _texture.Sample(smp, _uv);
    ret += gaussianWeights[0] * ret;
    
    // â°ï˚å¸
    for (int i = 1; i < 8; ++i)
    {
        ret += gaussianWeights[i >> 2][i % 4] * _texture.Sample(smp, _uv + float2(i * dx, 0))/256;
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3ÇÃï¿Ç—ÅBéüÇÃçsÇÕ-iÇÊÇË-1Å`-8Ç‹Ç≈
        ret += gaussianWeights[i >> 2][i % 4] * _texture.Sample(smp, _uv + float2(-i * dx, 0)) / 256;
    }
    
    // ècï˚å¸
    for (int j = 1; j < 8; ++j)
    {
        ret += gaussianWeights[j >> 2][j % 4] * _texture.Sample(smp, _uv + float2(0, j * dy)) / 256;
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3ÇÃï¿Ç—ÅBéüÇÃçsÇÕ-jÇÊÇË-1Å`-8Ç‹Ç≈
        ret += gaussianWeights[j >> 2][j % 4] * _texture.Sample(smp, _uv + float2(0, -j * dy)) / 256;
    }
    
    return ret;
}


float4 ps_main(vsOutput input) : SV_TARGET
{
    if(!sw)
    {
        return tex.Sample(smp, input.texCoord);
    }
    
    float w, h, levels;
    tex.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    
    float4 blur = /*shadowRendering.Sample(smp, input.texCoord)*/SimpleGaussianBlur(tex, smp, input.texCoord);
    //float4 blur = Get5x5GaussianBlur(shadowRendering, smp, input.texCoord, dx, dy);
    return blur;
}
RWTexture2D<float> bluredShadowmap; // UAVオブジェクト

cbuffer GaussianWeight : register(b0) // gaussian weight
{
    float4 gaussianWeights[2];
};

Texture2D<float> shadowMap : register(t0);
SamplerState smp : register(s0); // No.0 sampler

float GaussianBlur(Texture2D<float> _texture, SamplerState _smp, float2 _uv, float dx, float dy)
{
    float ret = 0;
    
    // 横方向
    for (int i = 0; i < 8; ++i)
    {
        ret += gaussianWeights[i >> 2][i % 4] * _texture.SampleLevel(smp, _uv + float2(i * dx, 0), 0);
        ret += gaussianWeights[i >> 2][i % 4] * _texture.SampleLevel(smp, _uv + float2(-i * dx, 0), 0);
    }
    
    // 縦方向
    for (int j = 0; j < 8; ++j)
    {
        ret += gaussianWeights[j >> 2][j % 4] * _texture.SampleLevel(smp, _uv + float2(0, j * dy), 0);
        ret += gaussianWeights[j >> 2][j % 4] * _texture.SampleLevel(smp, _uv + float2(0, -j * dy), 0);
    }
    
    return ret.x / 2;
}

float SimpleGaussianBlur(Texture2D<float> _texture, SamplerState _smp, float2 _uv, float dx, float dy)
{
    float ret = float4(0, 0, 0, 0);
    
    // highest
    ret += _texture.SampleLevel(smp, _uv + float2(-2 * dx, 2 * dy), 0) * 1;
    ret += _texture.SampleLevel(smp, _uv + float2(-1 * dx, 2 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(0 * dx, 2 * dy), 0) * 6;
    ret += _texture.SampleLevel(smp, _uv + float2(1 * dx, 2 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(2 * dx, 2 * dy), 0) * 1;
    // high
    ret += _texture.SampleLevel(smp, _uv + float2(-2 * dx, 1 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(-1 * dx, 1 * dy), 0) * 16;
    ret += _texture.SampleLevel(smp, _uv + float2(0 * dx, 1 * dy), 0) * 24;
    ret += _texture.SampleLevel(smp, _uv + float2(1 * dx, 1 * dy), 0) * 16;
    ret += _texture.SampleLevel(smp, _uv + float2(2 * dx, 1 * dy), 0) * 4;
    // middle
    ret += _texture.SampleLevel(smp, _uv + float2(-2 * dx, 0 * dy), 0) * 6;
    ret += _texture.SampleLevel(smp, _uv + float2(-1 * dx, 0 * dy), 0) * 24;
    ret += _texture.SampleLevel(smp, _uv + float2(0 * dx, 0 * dy), 0) * 36;
    ret += _texture.SampleLevel(smp, _uv + float2(1 * dx, 0 * dy), 0) * 24;
    ret += _texture.SampleLevel(smp, _uv + float2(2 * dx, 0 * dy), 0) * 6;
    // low
    ret += _texture.SampleLevel(smp, _uv + float2(-2 * dx, -1 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(-1 * dx, -1 * dy), 0) * 16;
    ret += _texture.SampleLevel(smp, _uv + float2(0 * dx, -1 * dy), 0) * 24;
    ret += _texture.SampleLevel(smp, _uv + float2(1 * dx, -1 * dy), 0) * 16;
    ret += _texture.SampleLevel(smp, _uv + float2(2 * dx, -1 * dy), 0) * 4;
    // lowest
    ret += _texture.SampleLevel(smp, _uv + float2(-2 * dx, -2 * dy), 0) * 1;
    ret += _texture.SampleLevel(smp, _uv + float2(-1 * dx, -2 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(0 * dx, -2 * dy), 0) * 6;
    ret += _texture.SampleLevel(smp, _uv + float2(1 * dx, -2 * dy), 0) * 4;
    ret += _texture.SampleLevel(smp, _uv + float2(2 * dx, -2 * dy), 0) * 1;
 
    return ret / 256;
}

[numthreads(16, 16, 1)]
void cs_main( uint3 DTid : SV_DispatchThreadID )
{
    // レイの高度(u 0～100km)と、レイ方向を90°とした時に太陽がどの角度にあるか(v -PI/2 ～ PI/2)をベースに、S(x,li) = Vis(li) * T(x,x+tatmoli)の結果をLUTにする
    
    float width, height;
    bluredShadowmap.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float x = (DTid.x + 0.5) / width;
    float y = (DTid.y + 0.5) / height;
    float2 uv = float2(x, y);
    
    float dx = 1.0f / width;
    float dy = 1.0f / height;
    
    float result = SimpleGaussianBlur(shadowMap, smp, uv, dx, dy);

    bluredShadowmap[DTid.xy] = result; // x:スレッドID xが最小(rayHeight=0)～最大(rayHeight:100) yが最小(sunTheta=-PI/2)～最大(sunTheta:PI/2)の計算結果に対応している。

}
#include "IntegrationHeader.hlsli"
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

PixelOutput ps_main(vsOutput input) : SV_TARGET
{
    PixelOutput result;
    //float depth;
    float threadOneDepth = sponzaDepthmap.Sample(smp, input.uv);
    float threadTwoDepth = connanDepthmap.Sample(smp, input.uv);
    
    float4 sun = float4(0, 0, 0, 0);
    
    if (threadOneDepth == threadTwoDepth == 1)
    {
        result.color = SimpleGaussianBlur(sky, smp, input.uv /*, dx, dy*/);
        sun = SimpleGaussianBlur(sunTex, smp, input.uv /*, dx, dy*/);
    }
    else if (threadOneDepth < threadTwoDepth)
    {
        result.color = tex.Sample(smp, input.uv);
        result.normal = normal1.Sample(smp, input.uv);
        /*result.depth*/
        //depth = float4(threadOneDepth, threadOneDepth, threadOneDepth, 1);
    }
    else
    {
        result.color = tex2.Sample(smp, input.uv);
        result.normal = normal2.Sample(smp, input.uv);
        /*result.depth*/
        //depth = float4(threadTwoDepth, threadTwoDepth, threadTwoDepth, 1);
    }
   
    float3 col = result.color.rgb;
    col = tonemap(col);
    col = saturate(pow(col, 1.0f / 2.6f/*2.2f*/));
    
    float4 imgui = imguiWindow.Sample(smp, input.uv) / 2.0f;
    //if(imgui.r != 0)
    //{
    //    col = imgui;
    //}
    
    //float4 sun = SimpleGaussianBlur(sunTex, smp, input.uv /*, dx, dy*/);
    result.color.rgb = col;
    result.color += /*imgui + */sun;
    result.imgui = imgui;
    //result.depth = depth;
    return result;
}
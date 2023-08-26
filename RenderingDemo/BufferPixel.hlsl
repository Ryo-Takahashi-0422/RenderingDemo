#include "BufferHeader.hlsli"

// entry point for BufferShaderCompile
float4 psBuffer(Output input) : SV_TARGET
{
    float4 result = { 0, 0, 0, 0 };    
    result = model.Sample(smp, input.uv); // default
    
    if (isFoV)
    {
        result = FOVEffect(shrinkedModel, smp, input.uv, 0.0f);
    }   
    
    if (isSSAO)
    {
        float ao = SSAOBlur(smp, input.uv);
        float4 aoValue = float4(ao, ao, ao, 1);
        //return saturate(aoValue);
        result *= saturate(aoValue); // AOPixelShader側で最後にSaturateしているにも関わらず1.0以上の値が返ってきている...なぜ？？？？
    }
    
    if (isBloom)
    {
        result += BloomEffect(shrinkedbloommap, input.uv);
    }    
        
    return result + imgui.Sample(smp, input.uv);
}

float4 MakePAL(float4 col)
{
    float Y = dot(col.rgb, float3(0.299, 0.587, 0.114));
    return float4(Y, Y, Y, 1);
}

float4 RevrseColor(float4 col)
{
    return float4(float3(1.0f, 1.0f, 1.0f) - col.rgb, col.a);
}

float4 DownColorGradation(float4 col)
{
    return float4(col.rgb - fmod(col.rgb, 0.25f), col.a);
}

float4 Emboss(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy)
{
    float4 ret = float4(0, 0, 0, 0);
    
    // up-left, up, up-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, offset * dy)) * 2;
    ret += _texture.Sample(_smp, _uv + float2(0, offset * dy));
    ret += _texture.Sample(_smp, _uv) * 0;
    
    // left, center, right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, 0));
    ret += _texture.Sample(_smp, _uv);
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, 0)) * -1;
    
    //down-left, down, down-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, -offset * dy)) * 0;
    ret += _texture.Sample(_smp, _uv + float2(0, -offset * dy)) * -1;
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, -offset * dy)) * -2;
    
    return ret;
}

float4 Sharpness(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy)
{
    float4 ret = float4(0, 0, 0, 0);
    
    // up-left, up, up-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, offset * dy)) * 0;
    ret += _texture.Sample(_smp, _uv + float2(0, offset * dy)) * -1;
    ret += _texture.Sample(_smp, _uv) * 0;
    
    // left, center, right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, 0)) * -1;
    ret += _texture.Sample(_smp, _uv) * 4;
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, 0)) * -1;
    
    //down-left, down, down-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, -offset * dy)) * 0;
    ret += _texture.Sample(_smp, _uv + float2(0, -offset * dy)) * -1;
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, -offset * dy)) * 0;
    
    float Y = dot(ret.rgb, float3(0.299, 0.587, 0.114));
    Y = pow(1.0f - Y, 10.0f);
    Y = step(0.2, Y);
    return float4(Y, Y, Y, 1);
}

float4 Get5x5GaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv, float dx, float dy)
{
    float4 ret = float4(0, 0, 0, 0);
    ret = _texture.Sample(smp, _uv);
    ret += bkweights[0] * ret;
    
    // 横方向
    for (int i = 1; i < 8; ++i)
    {
        ret += bkweights[i >> 2][i % 4] * _texture.Sample(smp, _uv + float2(i * dx, 0));
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3の並び。次の行は-iより-1～-8まで
        ret += bkweights[i >> 2][i % 4] * _texture.Sample(smp, _uv + float2(-i * dx, 0));
    }
    
    // 縦方向
    for (int j = 1; j < 8; ++j)
    {
        ret += bkweights[j >> 2][j % 4] * _texture.Sample(smp, _uv + float2(0, 1 * dy));
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3の並び。次の行は-jより-1～-8まで
        ret += bkweights[j >> 2][j % 4] * _texture.Sample(smp, _uv + float2(0, -j * dy));
    }
    
    return ret;
}

float4 Gauss(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect)
{
    float4 ret = tex.Sample(smp, uv);

    float l1 = -dx, l2 = -2 * dx;
    float r1 = dx, r2 = 2 * dx;
    float u1 = -dy, u2 = -2 * dy;
    float d1 = dy, d2 = 2 * dy;
    l1 = max(uv.x + l1, rect.x) - uv.x;
    l2 = max(uv.x + l2, rect.x) - uv.x;
    r1 = min(uv.x + r1, rect.z - dx) - uv.x;
    r2 = min(uv.x + r2, rect.z - dx) - uv.x;

    u1 = max(uv.y + u1, rect.y) - uv.y;
    u2 = max(uv.y + u2, rect.y) - uv.y;
    d1 = min(uv.y + d1, rect.w - dy) - uv.y;
    d2 = min(uv.y + d2, rect.w - dy) - uv.y;

    return float4((
		  tex.Sample(smp, uv + float2(l2, u2)).rgb
		+ tex.Sample(smp, uv + float2(l1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, u2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, u2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, u2)).rgb

		+ tex.Sample(smp, uv + float2(l2, u1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, u1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, u1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, u1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, 0)).rgb * 6
		+ tex.Sample(smp, uv + float2(l1, 0)).rgb * 24
		+ ret.rgb * 36
		+ tex.Sample(smp, uv + float2(r1, 0)).rgb * 24
		+ tex.Sample(smp, uv + float2(r2, 0)).rgb * 6

		+ tex.Sample(smp, uv + float2(l2, d1)).rgb * 4
		+ tex.Sample(smp, uv + float2(l1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(0, d1)).rgb * 24
		+ tex.Sample(smp, uv + float2(r1, d1)).rgb * 16
		+ tex.Sample(smp, uv + float2(r2, d1)).rgb * 4

		+ tex.Sample(smp, uv + float2(l2, d2)).rgb
		+ tex.Sample(smp, uv + float2(l1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(0, d2)).rgb * 6
		+ tex.Sample(smp, uv + float2(r1, d2)).rgb * 4
		+ tex.Sample(smp, uv + float2(r2, d2)).rgb
	) / 256.0f, ret.a);
}

float4 AverageBlur(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy)
{
    float4 ret = float4(0, 0, 0, 0);
    
    // up-left, up, up-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, offset * dy));
    ret += _texture.Sample(_smp, _uv + float2(0, offset * dy));
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, offset * dy));
    
    // left, center, right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, 0));
    ret += _texture.Sample(_smp, _uv);
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, 0));
    
    //down-left, down, down-right
    ret += _texture.Sample(_smp, _uv + float2(-offset * dx, -offset * dy));
    ret += _texture.Sample(_smp, _uv + float2(0, -offset * dy));
    ret += _texture.Sample(_smp, _uv + float2(offset * dx, -offset * dy));
    
    return ret / 9.0f;
}

float4 SimpleGaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv/*, float dx, float dy*/)
{
    float4 ret = float4(0, 0, 0, 0);
    
    float w, h, levels;
    model.GetDimensions(0, w, h, levels);
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

float4 NormalmapEffect(float2 _uv)
{
    float4 ret = float4(0, 0, 0, 0);
    float2 normalTex = normalmap.Sample(smp, _uv);
    normalTex = normalTex * 2.0f - 1.0f; // 法線マップは法線方向Nx,Ny,Nzをそれぞれ1+Nx(Ny,Nz)/2として画像データとしているので、法線方向データに戻す
    
    for (int i = 1; i < 8; ++i)
    {
        ret += bkweights[i >> 2][i % 4] * model.Sample(smp, _uv + normalTex * 0.1f);
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3の並び。次の行は-iより-1～-8まで
        ret += bkweights[i >> 2][i % 4] * model.Sample(smp, _uv + normalTex * 0.1f);
    }
    
    //return float4(ret.rgb / 2, ret.a);
    
    ret = model.Sample(smp, _uv + normalTex * 0.1f) + tex.Sample(smp, _uv/* + normalTex * 0.5f*/);
    
    return ret;
}

float4 DefferedShading(float2 _uv)
{
    float4 nm = multinormalmap.Sample(smp, _uv);
    nm = nm * 2.0f - 1.0f;
    float3 light = normalize(float3(1.0f, -1.0f, 1.0f));
    const float ambient = 0.7f;
    
    // dot(nm.xyz, -light)) is cosθ between normalized reflection(-light) direction and normal direction of vertex
    float diffB = max(saturate(dot(nm.xyz, -light)), ambient);    
    
    return model.Sample(smp, _uv) * float4(diffB, diffB, diffB, 1);
}

float4 BloomEffect(Texture2D _texture, float2 _uv)
{
    float w, h, levels;
    model.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    float4 bloomAccum = float4(0, 0, 0, 0);
    float2 uvSize = float2(0.5, 0.5);
    float2 uvOfst = float2(0, 0);
    
    for (int i = 0; i < 3; ++i)
    {
        bloomAccum += Get5x5GaussianBlur(_texture, smp, _uv * uvSize + uvOfst, dx, dy);
        uvOfst.y += uvSize.y;
        uvSize *= 0.5f;
    }
    
    // all 1 = white bloom. xy /= 10 is blue cool bloom.
    bloomAccum.xyz /= 10;
    
    return /*model.Sample(smp, _uv) + */float4(saturate(bloomAccum.xyz * bloomCol), bloomAccum.w);
}

float4 FOVEffect(Texture2D _texture, SamplerState _smp, float2 _uv, float focusDistance)
{
    float4 ret = (0, 0, 0, 1);
    float w, h, levels;
    model.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    
    float y = focusDistance;
    float dp = depthmap.Sample(smp, _uv);
    dp = pow(dp, 20.0); // Calculation between small values (y-dp) isn't effective. 'dp' should be emphasized enough before (y-dp).
    //return float4(dp, dp, dp, 1);
    
    float depthDiff = abs(y - dp);
    //depthDiff = pow(depthDiff, 20.0f);
    
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);

    float4 retColor[2];    
    retColor[0] = model.Sample(smp, _uv);

    //ret = SimpleGaussianBlur(model, smp, input.uv, dx, dy);
    
    ret = SimpleGaussianBlur(shrinkedModel, smp, _uv * uvSize + uvOfst/*, dx, dy*/);

    retColor[1] = ret;
    
    depthDiff = saturate(depthDiff * 2);
    return lerp(retColor[0], retColor[1], depthDiff);
}

float SSAOBlur(SamplerState _smp, float2 _uv)
{
    float ret = 0.0f;
    
    float w, h, levels;
    model.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    ret = aomap.Sample(smp, _uv);
    ret += bkweights[0] * ret;
    
    // 横方向
    for (int i = 1; i < 8; ++i)
    {
        ret += bkweights[i >> 2][i % 4] * aomap.Sample(smp, _uv + float2(i * dx, 0));
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3の並び。次の行は-iより-1～-8まで
        ret += bkweights[i >> 2][i % 4] * aomap.Sample(smp, _uv + float2(-i * dx, 0));
    }
    
    // 縦方向
    for (int j = 1; j < 8; ++j)
    {
        ret += bkweights[j >> 2][j % 4] * aomap.Sample(smp, _uv + float2(0, 1 * dy));
        //0000, 1
        //0000, 2
        //0000, 3
        //0001, 0
        //0001, 1
        //0001, 2
        //0001, 3の並び。次の行は-jより-1～-8まで
        ret += bkweights[j >> 2][j % 4] * aomap.Sample(smp, _uv + float2(0, -j * dy));
    }
    
    return ret;
}
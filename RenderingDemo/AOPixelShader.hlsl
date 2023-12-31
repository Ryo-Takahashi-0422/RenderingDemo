#include <AOShaderHeader.hlsli>

float SsaoPs(Output input) : SV_TARGET
{
    float result = 0.0f;
    
    float w, h, levels;
    depthmap.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;
 
    float4 rpos = (0, 0, 0, 0);

    float dp = depthmap.Sample(smp, float2(input.svpos.x / w, input.svpos.y / h));
    //return dp;
    // SSAO

    float div = 0.0f;
    float ao = 0.0f;
    float3 norm = normalize(input.norm.xyz);
    const int trycnt = 256;
    const float radius = 15.0f;
    
    if(dp < 1.0f)
    {
        for (int i = 0; i < trycnt; ++i)
        {            
            float rnd1 = random(float2(i * dx, i * dy)) * 2 - 1;
            float rnd2 = random(float2(rnd1, i * dy)) * 2 - 1;
            float rnd3 = random(float2(rnd2, rnd1)) * 2 - 1;
            float3 omega = normalize(float3(rnd1, rnd2, rnd3));
            omega = normalize(omega);
            
            ///////
            norm.x += omega.x * radius;
            norm.y += omega.y * radius;
            norm.z += omega.z * radius;

            
            // 乱数の結果法線の反対側に向いていたら反転
            float dt = dot(norm, omega);
            float sgn = sign(dt);
            omega *= sign(dt);
            dt *= sgn; // 正の値にしてcosθを得る            
            div += dt; // 遮断を考えない結果を加算する 
            
            rpos.x = input.svpos.x + omega.x * radius;
            rpos.y = input.svpos.y + omega.y * radius; // omega.yを足すとなぜかrpos.zに悪影響が出て深度チェックが上手くいかない...なぜ？？？
            rpos.z = input.svpos.z;
            rpos.w = input.svpos.w;
            
            // 計算結果が現在の場所の深度より奥に入っているなら遮断されているので加算する
            // x > y = 1, x < y = 0
            ao += step(depthmap.Sample(smp, float2(rpos.x / w, rpos.y / h)), rpos.z) * dt;
        }
        
        ao /= div;
        result = saturate((1.0f - ao) * 1.6f);
        return result/*saturate((1.0f - ao) * 1.6f)*/;
    }
    
    return 1.0f;
}

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}
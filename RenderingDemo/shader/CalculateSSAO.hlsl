RWTexture2D<float4> ssao; // UAVオブジェクト
Texture2D<float4> normalmap : register(t0);
Texture2D<float> depthmap : register(t1);
SamplerState smp : register(s0); // No.0 sampler

cbuffer Matrix4Cal : register(b0) // gaussian weight
{
    matrix view;
    matrix invView;
    matrix proj;
    matrix invProj;
    bool isDraw;
};

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

[numthreads(16, 16, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
    float result = 0.0f;
    float width, height;
    normalmap.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float x = (DTid.x + 0.5) / width;
    float y = (DTid.y + 0.5) / height;
    float2 uv = float2(x, y);
    float dp = depthmap.SampleLevel(smp, float2(uv.x, uv.y), 0);
    
    if (!isDraw)
    {
        ssao[DTid.xy] = float4(1, 1, 1, 0);
    }
    // 遠くのオブジェクトは対象外とする
    else if (dp > 0.975f)
    {
        ssao[DTid.xy] = float4(1, 1, 1, 0);
    }

    else
    {
    
        float4 respos = mul( /*mul(*/invProj /*, invView)*/, float4(uv * float2(2, -2) + float2(-1, 1), dp, 1));
        respos.xyz = respos.xyz / respos.w;
    
        float dx = 1.0f / width;
        float dy = 1.0f / height;
    
        float4 rpos = (0, 0, 0, 0);

        float div = 0.0f;
        float ao = 0.0f;
        float3 norm = normalize((normalmap.SampleLevel(smp, float2(uv.x, uv.y), 0).xyz * 2) - 1);
        const int trycnt = 64;
        const float radius = 0.05f;
    
        if (dp < 1.0f)
        {
            for (int i = 0; i < trycnt; ++i)
            {
                float rnd1 = random(float2(i * dx, i * dy)) * 2 - 1;
                float rnd2 = random(float2(rnd1, i * dy)) * 2 - 1;
                float rnd3 = random(float2(rnd2, rnd1)) * 2 - 1;
                float3 omega = normalize(float3(rnd1, rnd2, rnd3));
                omega = normalize(omega);
            
            // 乱数の結果法線の反対側に向いていたら反転
                float dt = dot(norm, omega);
                float sgn = sign(dt);
                omega *= sign(dt);
                dt *= sgn; // 正の値にしてcosθを得る            
                div += dt; // 遮断を考えない結果を加算する 
            
                float4 rpos = mul(proj, /* mul(view,*/float4(respos.xyz + omega * radius, 1 /*)*/));
                rpos.xyz /= rpos.w;
            
                float3 oNorm = normalize(normalmap.SampleLevel(smp, (float2(rpos.x, rpos.y) + float2(1, -1)) * float2(0.5, -0.5), 0).xyz);
                oNorm = clamp(oNorm, 0, 1);
                float normDiff = (1.0 - dot(oNorm, norm));
            // 計算結果が現在の場所の深度より奥に入っているなら遮断されているので加算する
            // x > y = 1, x < y = 0
                float sampleDepth = depthmap.SampleLevel(smp, (rpos.xy + float2(1, -1)) * float2(0.5f, -0.5f), 0);
                float depthDifference = abs(sampleDepth - rpos.z);
                if (depthDifference > 0.001f)
                {
                
                }
                else
                {
                    ao += step(sampleDepth /* + 0.0005f*/ + 0.0004f, rpos.z) * dt * normDiff /* * (1.0 - smoothstep(0.000002f, 0.0007f, depthDifference))*/;
                }
            
            }
        
            ao /= (float) trycnt;
        //ao = pow(ao, 4);

        }
        
        result = 1.0f - ao;
    //result = pow(result, 4);
        result *= result;
        ssao[DTid.xy] = float4(result, result, result, 0);
    }
}
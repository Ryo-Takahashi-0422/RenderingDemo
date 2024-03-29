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
    
    float x = (DTid.x + 0.5f) / width;
    float y = (DTid.y + 0.5f) / height;
    float2 uv = float2(x, y);
    float dp = depthmap.SampleLevel(smp, float2(uv.x, uv.y), 0.0f);
    
    if (!isDraw)
    {
        ssao[DTid.xy] = float4(1.0f, 1.0f, 1.0f, 0.0f);
    }
    // 遠くのオブジェクトは対象外とする
    else if (dp > 0.975f)
    {
        ssao[DTid.xy] = float4(1.0f, 1.0f, 1.0f, 0.0f);
    }

    else
    {    
        float4 respos = mul( /*mul(*/invProj /*, invView)*/, float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
        respos.xyz /= respos.w;
    
        float dx = 1.0f / width;
        float dy = 1.0f / height;
    
        float4 rpos = (0, 0, 0, 0);

        float div = 0.0f;
        float ao = 0.0f;
        float3 norm = normalize(normalmap.SampleLevel(smp, float2(uv.x, uv.y), 0.0f).xyz * 2.0f - 1.0f);
        norm = mul(view, norm);
        const int trycnt = 48;
        const float radius = 0.1f;
    
        if (dp < 1.0f)
        {
            for (int i = 0; i < trycnt; ++i)
            {
                float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
                float rnd2 = random(float2(rnd1, i * dy)) * 2.0f - 1.0f;
                float rnd3 = random(float2(rnd2, rnd1)) * 2.0f - 1.0f;
                float3 omega = normalize(float3(rnd1, rnd2, rnd3));
                omega = normalize(omega);
                
                // Create TBN matrix
                //float3 tangent = normalize(omega - norm * dot(omega, norm));
                //float3 bitangent = cross(tangent, norm);
                //float3x3 TBN = float3x3(tangent, bitangent, norm);
                //omega = mul(TBN, omega);
                
                // 乱数の結果法線の反対側に向いていたら反転
                float dt = dot(norm, omega);
                float sgn = sign(dt);
                omega *= sign(dt);
                dt *= sgn; // 正の値にしてcosθを得る            
                div += dt; // 遮断を考えない結果を加算する 
            
                float4 rpos = mul(proj, /* mul(view,*/float4(respos.xyz + omega * radius, 1.0f /*)*/));
                rpos.xyz /= rpos.w;                
            
                float3 oNorm = normalize(normalmap.SampleLevel(smp, (float2(rpos.x, rpos.y) + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f), 0.0f).xyz);
                //oNorm = mul(view, float4(oNorm, 1));
                //oNorm = clamp(oNorm, 0.0f, 1.0f);
                oNorm = mul(view, oNorm);
                float normDiff = (1.0f - abs(dot(norm, oNorm)));
                normDiff = smoothstep(0.0f, 1.0f, normDiff);
                //normDiff = smoothstep(0.0f, 1.0f, normDiff);
                //normDiff = smoothstep(0.0f, 1.0f, normDiff);
                //dt = smoothstep(0.0f, 1.0f, dt);
                dt = smoothstep(0.0f, 1.0f, dt);
            // 計算結果が現在の場所の深度より奥に入っているなら遮断されているので加算する
            // x > y = 1, x < y = 0
                float sampleDepth = depthmap.SampleLevel(smp, (rpos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f), 0.0f);
                float depthDifference = abs(sampleDepth - /*rpos.z*/dp);
                
                //float rangeCheck = smoothstep(0.0f, 1.0f, radius / length(dp - sampleDepth));
                if (depthDifference <= 0.0028f)
                {
                    ao += smoothstep(0, 1, step(sampleDepth /* + 0.0005f*/ + 0.0001f, dp) * dt * normDiff);
                }           
            }
        
            ao /= (float) trycnt;
        //ao = pow(ao, 4);

        }
        
        result = 1.0f - ao;
        result = pow(result, 3);
        //result *= result;
        ssao[DTid.xy] = float4(result, result, result, 0.0f);
    }
}
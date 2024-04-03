RWTexture2D<float4> ssao; // UAVオブジェクト
Texture2D<float4> normalmap : register(t0);
Texture2D<float> depthmap : register(t1);
SamplerState smp : register(s0); // No.0 sampler

cbuffer Matrix4Cal : register(b0) // gaussian weight
{
    //matrix view;
    //matrix invView;
    matrix proj;
    matrix invProj;
    bool isDraw;
};

// TODO:ノイズテクスチャから乱数作成
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
    //else if (dp > 0.975f)
    //{
    //    ssao[DTid.xy] = float4(1.0f, 1.0f, 1.0f, 0.0f);
    //}

    else
    {    
        float4 respos = mul( /*mul(*/invProj /*, invView)*/, float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
        respos.xyz /= respos.w;
    
        float dx = 1.0f / width;
        float dy = 1.0f / height;
    
        float4 rpos = (0, 0, 0, 0);

        float ao = 0.0f;
        float3 oriNorm = normalmap.SampleLevel(smp, float2(uv.x, uv.y), 0.0f);
        // 床とキャラクターとのSSAOを計算しないための処理1
        // この処理のためにキャラクターの法線出力値は全て0としている
        if (oriNorm.x + oriNorm.y + oriNorm.z == 3.0f)
        {
            dp = 1.0f;
        }
        float3 norm = normalize(oriNorm.xyz * 2.0f - 1.0f);
        //norm = mul(view, norm);
        const int trycnt = 48;
        const float radius = 0.12f;
    
        if (dp < 1.0f)
        {
            for (int i = 0; i < trycnt; ++i)
            {
                float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
                float rnd2 = random(float2(i * rnd1, i * dy)) * 2.0f - 1.0f;
                float rnd3 = random(float2(rnd2, rnd1)) * 2.0f - 1.0f;               
                float3 omega = normalize(float3(rnd1, rnd2, rnd3));

                // Create TBN matrix
                //float3 tangent = normalize(omega - norm * dot(omega, norm));
                //float3 bitangent = cross(tangent, norm);
                //float3x3 TBN = float3x3(tangent, bitangent, norm);
                //omega = mul(TBN, omega);
                
                // 乱数の結果法線の反対側に向いていたら反転
                float dt = dot(norm, omega);
                float sgn = sign(dt);
                omega *= sgn;
                dt *= sgn; // 正の値にしてcosθを得る            
            
                float4 rpos = mul(proj, float4(respos.xyz + omega * radius, 1.0f));
                rpos.xyz /= rpos.w;                
            
                float2 sPos = (float2(rpos.x, rpos.y) + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
                float3 sOriNorm = normalmap.SampleLevel(smp, sPos, 0.0f).xyz;
                float3 sNorm = normalize(sOriNorm * 2.0f - 1.0f);

                //sNorm = mul(view, sNorm);
                float normDiff = (1.0f - abs(dot(norm, sNorm)));

                // 計算結果が現在の場所の深度より奥に入っているなら遮断されているので加算する
                float sampleDepth = depthmap.SampleLevel(smp, (rpos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f), 0.0f);
                float depthDifference = abs(sampleDepth - /*rpos.z*/dp);
                
                // 床とキャラクターとのSSAOを計算しないための処理2
                if (sOriNorm.x + sOriNorm.y + sOriNorm.z == 3.0f)
                {
                    depthDifference = 1.0f;
                }
                if (depthDifference <= /*0.0028f*/0.003f)
                {
                    ao += step(sampleDepth + 0.000001f, dp) * (1.0f - dt) * normDiff;
                }           
            }        
            ao /= (float) trycnt;
        }
        
        result = 1.0f - ao;
        result = pow(result, 2);
        ssao[DTid.xy] = /*float4(result, result, result, 0.0f)*/result;
    }
}
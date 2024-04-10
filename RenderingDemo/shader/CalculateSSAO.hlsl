RWTexture2D<float4> ssao; // UAVオブジェクト
Texture2D<float4> normalmap : register(t0);
Texture2D<float> depthmap : register(t1);
SamplerState smp : register(s0); // No.0 sampler

#define PI 3.14
#define RAY_EPSILON 0.001

cbuffer Matrix4Cal : register(b0) // gaussian weight
{
    matrix view;
    matrix rotation;
    matrix proj;
    matrix invProj;
    bool isDraw;
};

// TODO:ノイズテクスチャから乱数作成
float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float3 getCosHemisphereSample(float randSeed1, float randSeed2, float randSeed3, float3 hitNorm)
{
// 2つのランダムな数値を取得
    float2 randVal = float2(randSeed1, randSeed2);
// 法線に垂直なベクトルを取る（最後に利用する）
    //float3 bitangent = getPerpendicularVector(hitNorm);
    //float3 tangent = cross(bitangent, hitNorm);
    //hitNorm = mul(view, hitNorm);
    float3 omega = normalize(float3(randSeed1, randSeed2, randSeed3));
    float3 tangent = normalize(omega - hitNorm * dot(omega, hitNorm));
    float3 bitangent = cross(tangent, hitNorm);
// ディスク上に一様にサンプリング
    randVal.x *= sign(randVal.x);
    //randVal.y *= sign(randVal.y);
    float r = sqrt(randVal.x);
    float phi = 2.0f * PI * randVal.y;
// 半球に射影する
    float x = r * cos(phi);
    float z = r * sin(phi);
    float y = sqrt(1.0 - randVal.x); // 1- r2
// 法線ベクトルの座標系に射影
    return x * tangent + y * hitNorm.xyz + z * bitangent;
}

float shootShadowRay(float3 position, float3 sampleDir, float minT, float maxT, float distance, float depth, float3 normal)
{
    float visibility = 0.0f;
    for (int i = 1; i < 5; ++i)
    {
        float4 samplePos = float4(float3(position + sampleDir * i * distance), 1.0f);
        samplePos = mul(proj, samplePos);
        samplePos.xyz /= samplePos.w;
        
        float sampleDepth = depthmap.SampleLevel(smp, (samplePos.xy + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f), 0.0f);
        
        float2 sPos = (float2(samplePos.x, samplePos.y) + float2(1.0f, -1.0f)) * float2(0.5f, -0.5f);
        float3 sOriNorm = normalmap.SampleLevel(smp, sPos, 0.0f).xyz;
        float3 sNorm = normalize(sOriNorm * 2.0f - 1.0f);
        sNorm.x *= sign(sNorm.x);
        sNorm.z *= sign(sNorm.z);
        //sNorm = mul(view, sNorm);
        float normDiff = (1.0f - abs(dot(normal, sNorm)));
        
        float depthDifference = abs(sampleDepth - /*rpos.z*/depth);
        if (sOriNorm.x + sOriNorm.y + sOriNorm.z == 3.0f)
        {
            depthDifference = 1.0f;
        }
        
        if (sampleDepth - 0.001f > samplePos.z && depthDifference <= /*0.0028f*/0.005f)
        {
            /*return 1.0f*/
            visibility += 1.0f / (4.0f * PI);
        }
    }

    return visibility;
}

float evaluateAO(float3 position, float3 normal, float randSeed1, float randSeed2, float randSeed3, float depth)
{
//    uint2 pixIdx = DispatchRaysIndex().xy; // レイインデックス x=0～1920, y=0～1080
//    uint2 numPix = DispatchRaysDimensions().xy; // ステージサイズ x=1920, y=1080
//// ランダムなシードを計算
    
//    uint randSeed = initRand(pixIdx.x + pixIdx.y * numPix.x, 100);
// 遮蔽度合い
    float visibility = 0.0f;
// 飛ばすレイの回数
    const int aoRayCount = 4;
    for (int i = 0; i < aoRayCount; ++i)
    {
// 法線を中心とした半球上のランダムなベクトルのサンプリング（コサイン重み付き分布）
        float3 sampleDir = getCosHemisphereSample(randSeed1, randSeed2, randSeed3,normal);
// シャドウレイを飛ばす
        float sampleVisibility = shootShadowRay(position, sampleDir, RAY_EPSILON, 10.0, 0.05, depth, normal);
//遮蔽度合い += サンプリングした値 × コサイン項 / 確率密度関数
        float NoL = /*saturate(*/dot(normal, sampleDir)/*)*/;
        float pdf = NoL / PI;
        visibility += sampleVisibility * NoL / pdf;
    }
// 平均を取る
    return (1 / PI) * (1 / float(aoRayCount)) * visibility;
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
    float dp = depthmap.SampleLevel(smp, uv, 0.0f);
    
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
        float4 respos = mul( /*mul(*/invProj /*, rotation)*/, float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp, 1.0f));
        respos.xyz /= respos.w;
    
        float dx = 1.0f / width;
        float dy = 1.0f / height;
    
        float4 rpos = (0, 0, 0, 0);
        matrix v = view;
        float ao = 0.0f;
        float3 oriNorm = normalmap.SampleLevel(smp, uv, 0.0f);
        // 床とキャラクターとのSSAOを計算しないための処理1
        // この処理のためにキャラクターの法線出力値は全て0としている
        if (oriNorm.x + oriNorm.y + oriNorm.z == 3.0f)
        {
            dp = 1.0f;
        }
        //oriNorm = mul(v, oriNorm);
        float3 norm = normalize(oriNorm.xyz * 2.0f - 1.0f);
        //norm = mul(v, norm);
        //norm.x *= sign(norm.x);
        norm.y *= sign(norm.y);
        //norm.z *= sign(norm.z);
        norm = normalize(norm);
        
        //norm = mul(v, norm);
        norm = normalize(norm);
        const int trycnt = 32;
        float radius = 0.02f;

        
        if (dp < 1.0f)
        {
            for (int i = 0; i < trycnt; ++i)
            {
                float rnd1 = random(float2(i * dx, i * dy)) * 2.0f - 1.0f;
                float rnd2 = random(float2(i * rnd1, i * dy)) * 2.0f - 1.0f;
                float rnd3 = random(float2(rnd2, rnd1)) * 2.0f - 1.0f;
                float3 omega = normalize(float3(rnd1, rnd2, rnd3));

                // RTAOテスト
                //ao += evaluateAO(respos.xyz, norm, rnd1, rnd2, rnd3, dp);
                // Create TBN matrix
                float3 tangent = normalize(omega - norm * dot(omega, norm));
                float3 biTangent = cross(norm, tangent);
                float3x3 TBN = float3x3(tangent, biTangent, norm);
                TBN = transpose(TBN);
                omega = mul(TBN, omega);
                
                //omega.x = abs(mul(TBN[0], omega.x));
                //omega.y = abs(mul(TBN[1], omega.y));
                //omega.z = abs(mul(TBN[2], omega.z));
                
                //omega.x = mul(TBN[0], omega.x);
                //omega.y = mul(TBN[1], omega.y);
                //omega.z = mul(TBN[2], omega.z);
                
                //omega.x = abs(mul(tangent, omega.x));
                //omega.y = abs(mul(biTangent, omega.y));
                //omega.z = abs(mul(norm, omega.z));
                
                //omega.x = mul(tangent, omega.x);
                //omega.y = mul(biTangent, omega.y);
                //omega.z = mul(norm, omega.z);
                omega = normalize(omega);
                
                // 乱数の結果法線の反対側に向いていたら反転
                float dt = dot(norm, omega);
                float sgn = sign(dt);
                //omega *= sgn;
                dt *= sgn; // 正の値にしてcosθを得る      
                
                // ピクセルのview zが大きい(遠く)ほどサンプリング半径を大きくする
                radius += saturate(respos.z / 5000);
                
                // カーテンに対しては一律半径
                if (respos.y <= -1.55f)
                {
                    radius = 0.15f;
                }
            
                float4 rpos = mul(proj, float4(respos.xyz + omega * radius, 1.0f));
                rpos.xy /= rpos.w;            
                rpos.xy = rpos.xy * 0.5f + 0.5f;
                rpos.y = 1.0 - rpos.y;
                float dp2 = depthmap.SampleLevel(smp, rpos.xy, 0.0f);
                float4 samplePos = mul(invProj, float4(rpos.xy * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), dp2, 1.0f));
                samplePos.xyz /= samplePos.w;
                float3 sOriNorm = normalmap.SampleLevel(smp, rpos.xy, 0.0f).xyz;
                //float3 sNorm = normalize(sOriNorm * 2.0f - 1.0f);
                //sNorm.x *= sign(sNorm.x);
                //sNorm.y *= sign(sNorm.y);
                //sNorm.z *= sign(sNorm.z);
                //norm = mul(TBN, norm);
                //sNorm = mul(TBN, sNorm);
                //sNorm = mul(v, sNorm);
                //sNorm = normalize(sNorm);
                //float normDiff = (1.0f - abs(dot(norm, sNorm)));

                // 計算結果が現在の場所の深度より奥に入っているなら遮断されているので加算する
                float sampleDepth = samplePos.z;
                float depthDifference = abs(dp2 - dp);
                
                // カーテンと床に安定してSSAOを生成するための処理
                if (respos.y <= -1.55f)
                {
                    depthDifference = 0.0f;
                }
                
                // 床に対してサンプリングさせない処理
                //if (samplePos.y <= -1.55f)
                //{
                //    depthDifference = 1.0f;
                //}
                
                // 床とキャラクターとのSSAOを計算しないための処理2
                if (sOriNorm.x + sOriNorm.y + sOriNorm.z == 3.0f || sOriNorm.z > 0.96f)
                {
                    depthDifference = 1.0f;
                }
                
                float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(respos.z - sampleDepth));
                if (depthDifference <= /*0.0028f*/0.015f + saturate(1.0f / respos.z)) // saturate...加算によって近くのビュー空間においてカメラ近くほど閾値を大きくして制限を弱め、カメラに近づくほどSSAOの計算がされない現象を回避している
                {
                    ao += step(sampleDepth + 0.1f, respos.z) /* * (1.0f - dt)*/ /* * normDiff*/;
                }
            }        
            ao /= (float) trycnt;
        }
        
        result = 1.0f - ao;
        result = smoothstep(0.0f, 1.0f, result);
        //result = pow(result, 2);
        ssao[DTid.xy] = result;
    }
}
RWTexture2D<float> integratedMap; // UAVオブジェクト
Texture2D<float> depthMap1 : register(t0);
Texture2D<float> depthMap2 : register(t1);
SamplerState smp : register(s0); // No.0 sampler

[numthreads(16, 16, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{  
    float width, height;
    depthMap1.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float x = (DTid.x + 0.5) / width;
    float y = (DTid.y + 0.5) / height;
    float2 uv = float2(x, y);
    
    float dx = 1.0f / width;
    float dy = 1.0f / height;
    
    float depth;
    float threadOneDepth = depthMap1.SampleLevel(smp, uv, 0);
    float threadTwoDepth = depthMap2.SampleLevel(smp, uv, 0);
    
    if (threadOneDepth == threadTwoDepth == 1)
    {
        depth = threadOneDepth;
    }
    else if (threadOneDepth < threadTwoDepth)
    {
        depth = threadOneDepth;
    }
    else
    {
        depth = threadTwoDepth;
    }
    
    //float result = SimpleGaussianBlur(depthMap, smp, uv, dx, dy);

    integratedMap[DTid.xy] = depth; // x:スレッドID xが最小(rayHeight=0)～最大(rayHeight:100) yが最小(sunTheta=-PI/2)～最大(sunTheta:PI/2)の計算結果に対応している。

}
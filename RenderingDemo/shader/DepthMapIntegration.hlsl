RWTexture2D<float> integratedMap; // UAV�I�u�W�F�N�g
Texture2D<float> shadowMap1 : register(t0);
Texture2D<float> shadowMap2 : register(t1);
SamplerState smp : register(s0); // No.0 sampler

[numthreads(16, 16, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
    // ���C�̍��x(u 0�`100km)�ƁA���C������90���Ƃ������ɑ��z���ǂ̊p�x�ɂ��邩(v -PI/2 �` PI/2)���x�[�X�ɁAS(x,li) = Vis(li) * T(x,x+tatmoli)�̌��ʂ�LUT�ɂ���
    
    float width, height;
    shadowMap1.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float x = (DTid.x + 0.5) / width;
    float y = (DTid.y + 0.5) / height;
    float2 uv = float2(x, y);
    
    float dx = 1.0f / width;
    float dy = 1.0f / height;
    
    float depth;
    float threadOneDepth = shadowMap1.SampleLevel(smp, uv, 0);
    float threadTwoDepth = shadowMap2.SampleLevel(smp, uv, 0);
    
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
    
    //float result = SimpleGaussianBlur(shadowMap, smp, uv, dx, dy);

    integratedMap[DTid.xy] = depth; // x:�X���b�hID x���ŏ�(rayHeight=0)�`�ő�(rayHeight:100) y���ŏ�(sunTheta=-PI/2)�`�ő�(sunTheta:PI/2)�̌v�Z���ʂɑΉ����Ă���B

}
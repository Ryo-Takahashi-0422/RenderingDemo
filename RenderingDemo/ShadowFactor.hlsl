#include "RaySphereIntersection.hlsl"
#include "AtmosphericModel.hlsli"
#include "Definition.hlsli"

#define STEP_CNT 1000

RWTexture2D<float4> shadowFactor; // UAV�I�u�W�F�N�g

[numthreads(16, 16, 1)]
void cs_main( uint3 DTid : SV_DispatchThreadID )
{
    // ���C�̍��x(u 0�`100km)�ƁA���C������90���Ƃ������ɑ��z���ǂ̊p�x�ɂ��邩(v -PI/2 �` PI/2)���x�[�X�ɁAS(x,li) = Vis(li) * T(x,x+tatmoli)�̌��ʂ�LUT�ɂ���
    
    float width, height;
    shadowFactor.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float rayHeight = lerp(0, atmosphereRadius - groundRadius, (DTid.x + 0.5f) / width);
    float2 rayPos = float2(0, rayHeight + groundRadius);
    float sunTheta = asin(lerp(-1, 1, (DTid.y + 0.5f) / height)); // ���C��90�������Ƃ����Ƃ��ɁA���z���ǂ̊p�x�ɂ��邩

    float2 sunDir = float2(cos(sunTheta), sin(sunTheta));
    float t = 0;
    
    if (!DiscriminateIntersectionWithCircle(rayPos, sunDir, groundRadius, t)) // Vis(li) ���z���n�\�ɉB��Ă��Ȃ��ꍇ
    {
        if (!DiscriminateIntersectionWithCircle(rayPos, sunDir, atmosphereRadius, t))
        {
            shadowFactor[DTid.xy] = float4(0, 0, 0, 1);
            return;
        }
    }
    
    float2 end = rayPos + sunDir * t; // ���C�����z�����֑�C���I�[�܂�
    float3 sumSigmaT = float3(0, 0, 0);
    
    for (int i = 0; i < STEP_CNT; ++i)
    {
        float2 currentRayPos = lerp(rayPos, end, (i + 0.5) / STEP_CNT);
        float h = length(currentRayPos) - groundRadius;
        float3 sigmaT = GetSigmaT(h);
        sumSigmaT += sigmaT; // ���t(x)
    }
    
    sumSigmaT *= (t / STEP_CNT); // ���t(x) * |dx|
    float3 transmittance = exp(-sumSigmaT); // e^-(���t(x) * |dx|)
    //transmittance.x *= 1.5f; // enhance red color

    shadowFactor[DTid.xy] = float4(transmittance, 1); // x:�X���b�hID x���ŏ�(rayHeight=0)�`�ő�(rayHeight:100) y���ŏ�(sunTheta=-PI/2)�`�ő�(sunTheta:PI/2)�̌v�Z���ʂɑΉ����Ă���B

}
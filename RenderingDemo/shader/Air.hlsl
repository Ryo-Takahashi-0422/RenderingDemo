#include "RaySphereIntersection.hlsli"
#include "AtmosphericModel.hlsli"

cbuffer Frustum : register(b1) // ���݃J�������`�悵�Ă���X�N���[�����W4���̃��[���h��Ԃɂ�����x�N�g��
{
    float4 topLeftFrustum;
    float4 topRightFrustum;
    float4 bottomLeftFrustum;
    float4 bottomRightFrustum;
};

cbuffer SkyLUTBuffer : register(b2)
{
    matrix world;
    matrix sunViewMatrix;
    matrix sunProjMatrix;
    float3 eyePos;
    float tDistance;
    float3 adjustedEyePos;
    float limitDistance;
    float3 sunDirection;
};

RWTexture3D<float4> AirTexture; // �o�͐�UAV�I�u�W�F�N�g
SamplerState smp : register(s0); // No.0 sampler
Texture2D<float> shadowMap : register(t0);
Texture2D<float3> shadowFactor : register(t1);

[numthreads(16, 16, 1)]
void cs_main(uint3 DTid : SV_DispatchThreadID)
{
    // �J�������烌�C���e�s�N�Z�������֔�΂��A���s��z�Ƃ���3D�e�N�X�`���ɃV���O���X�L���b�^�����O�̒l��݉����Ċi�[���Ă����B�X�L���b�^�����O�̌v�Z�����邩�̓��C�̈ʒu�ɂ�����z�l�𑾗z���猩��
    
    float width, height, depth;
    AirTexture.GetDimensions(width, height, depth);
    if (DTid.x >= width || DTid.y >= height)
        return;
    
    float3 currentPixelDir =
    normalize(
        lerp(lerp(topLeftFrustum, topRightFrustum, DTid.x),
             lerp(bottomLeftFrustum, bottomRightFrustum, DTid.x), DTid.y));    
    
    float startT = 0;
    float divDepth = tDistance / depth;
    float endT = min(divDepth, limitDistance);
    float maxT;
    float3 cameraPos3D = float3(0, adjustedEyePos.y + groundRadius, 0);
    if (!DiscriminateIntersectionWithSphere(cameraPos3D, currentPixelDir, groundRadius, maxT))
    {
        DiscriminateIntersectionWithSphere(cameraPos3D, currentPixelDir, atmosphereRadius, maxT);
    }
    
    // ����������@https://nimushiki.com/2018/10/09/1252/#fracsindotcoxy_float212989878233_437585453
    float rand = frac(sin(dot(
        float2(DTid.x + 0.5f, DTid.y + 0.5f), float2(12.9898, 78.233) * 2.0)) * 43758.5453);
    
    float3 scattering = float3(0, 0, 0);
    float3 sumSigmaT = float3(0, 0, 0);    
    
    for (int z = 0; z < depth;++z)
    {
        float nextT = lerp(startT, endT, rand);
        float3 rayPos = cameraPos3D + nextT * currentPixelDir; // ���C�ʒu�����ݏ������̃s�N�Z��������nextT���i�߂�
        
        // ���݂̃��C�����z���猩�ĉe�̒��ɂ��邩���肷��
        bool isShadow = false;
        if (!hasIntersectionWithSphere(rayPos, sunDirection, groundRadius))
        {
            float4 shadowPos = mul(mul(mul(sunProjMatrix, sunViewMatrix), world), float4(rayPos, 1)); // ���݂̃��C�ʒu���A���z���猩���J�����̎�������W�ɕϊ�
            shadowPos.xyz /= shadowPos.w; // xy��w�Ŋ����āA�����l�𑾗z���猩���J�����̎�������(-1�`+1)�Ɏ��߂Ă���
            float2 shadowUV = 0.5 + float2(0.5, -0.5) * shadowPos.xy; // x�͂��̂܂�0�`1�ɒ������Ay�͋�Ԃ̏�[��0�ɁA���[��1�ɂȂ�悤�ɒ����By�̉��[��1�Ȃ̂�v���[��1�ŁA�J�������猩���G�ɕϊ����ꂽ��Ԃ̃��C�ʒu��uv�ŕ\��

            float shadowZ = shadowMap.SampleLevel(smp, shadowUV, 0);
            float rayZ = shadowPos.z;
            isShadow = rayZ > shadowZ; // ���C��z�ʒu�����z���猩���f�v�X�}�b�v��蓯�ʒu�Ŏ擾����z�l���傫����΁A���C�ʒu�͎Օ�����ĉe�ł���
        }
        
        // ���݂̃��C�����z���猩�ĉe�łȂ��Ȃ�X�L���b�^�����O�̌v�Z���s��
        if (!isShadow)
        {
            float h = length(rayPos) - groundRadius;
            float3 sigmaS = GetSigmaS(h);
            float3 sigmaT = GetSigmaT(h);
            float3 deltaSigmaT = sigmaT * (nextT - startT); // T(c,x)
            float3 transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));
            float phaseTheta = dot(sunDirection, -currentPixelDir);
            float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
            float angleBetweenSunlightAndRay = PI / 2 - acos(dot(-sunDirection, normalize(rayPos)));
        
            float u, v;
            u = h / (atmosphereRadius - groundRadius);
            v = (sin(angleBetweenSunlightAndRay) + 1) * 0.5;
            float3 sf = shadowFactor.SampleLevel(smp, float2(u, v), 0); // S(x,li)�v�Z
        
            scattering += (nextT - startT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult * sf;
            sumSigmaT += deltaSigmaT;
        }

        startT = nextT;
        endT = max(endT + divDepth, limitDistance);
    }
    
    AirTexture[DTid.xyz] = float4(scattering, 1);
}
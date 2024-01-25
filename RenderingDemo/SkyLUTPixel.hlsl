#include "SkyLUTHeader.hlsli"

void rayMarching(inout float3 scattering, inout float3 sumSigmaT, float currentT, float nextT, float phaseTheta, float3 dir, float3 cameraPos)
{
    float3 pos = cameraPos + nextT * dir;
    float h = length(pos) - groundRadius;

    float3 sigmaS = GetSigmaS(h);
    float3 sigmaT = GetSigmaT(h);
    float3 deltaSigmaT = sigmaT * (nextT - currentT); // T(c,x)
    float3 transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));

    if (hasIntersectionWithSphere(pos, -sunDirection, atmosphereRadius))
    {
        float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
        float angleBetweenSunlightAndRay = PI / 2 - acos(dot(-sunDirection, normalize(pos)));
        
        float u, v;
        u = h / (atmosphereRadius - groundRadius);
        v = (sin(angleBetweenSunlightAndRay) + 1) * 0.5;
        float3 sf = shadowFactor.Sample(smp, float2(u, v));
        // S(x,li)�v�Z
        
        scattering += (nextT - currentT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult * sf;
    }

    sumSigmaT += deltaSigmaT;
}
float4 ps_main(vsOutput input) : SV_TARGET
{
    float phi = input.texCoord.x * 2 * PI; // 0~2PI
    float theta = (input.texCoord.y - 1.0f) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float2 twoDimDir = (cosTheta, sinTheta); // ����������xy���ʂŒ�`����B��͈�l�ł��邽�ߋ��ʍ��W�ɂ�����phi(x��z)�͍l�����Ȃ��B
    float2 cameraPos2D;
    cameraPos2D.x = 0;
    cameraPos2D.y = eyePos.y + groundRadius;
    float endT;
    if (!DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, groundRadius, endT))
    {
        DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, atmosphereRadius, endT);
    }
    //return float4(0.0f,1.0f,0.0f,1.0f);
    // ���ʍ��W�Ń��C�}�[�`���O
    float3 cameraPos3D = (0, 0, 0);
    cameraPos3D.y = eyePos.y + groundRadius;
    float3 dir = (cosTheta * cosPhi, sinTheta, cosTheta * sinPhi); // �X�N���[�����W�����ʍ��W�ɕϊ�����B���C�����͉𑜓x / 360���m�ۏo����B
    float3 scattering = (0, 0, 0);
    float3 sumSigmaT = (0, 0, 0);
    
    float deltaT = endT / stepCnt;
    float currentT = 0;
    float nextT = 0;
    float phaseTheta = dot(sunDirection, -dir);
    
    for (int i = 0; i < stepCnt; ++i)
    {
        nextT += deltaT;
        rayMarching(scattering, sumSigmaT, currentT, nextT, phaseTheta, dir, cameraPos3D);
        currentT = nextT;
    }
    
    return float4(scattering * sunIntensity * 10, 1);

}
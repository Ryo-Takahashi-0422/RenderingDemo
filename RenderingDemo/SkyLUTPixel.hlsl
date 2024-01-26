#include "SkyLUTHeader.hlsli"

void rayMarching(inout float3 scattering, inout float3 sumSigmaT, float currentT, float nextT, float phaseTheta, float3 dir, float3 cameraPos)
{
    float3 rayPos = cameraPos + nextT * dir;
    float h = length(rayPos) - groundRadius;

    float3 sigmaS = GetSigmaS(h);
    float3 sigmaT = GetSigmaT(h);
    float3 deltaSigmaT = sigmaT * (nextT - currentT); // T(c,x)
    float3 transmittanceFromRayToEye = exp(-(sumSigmaT + deltaSigmaT));

    if (!hasIntersectionWithSphere(rayPos, -sunDirection, groundRadius))
    {
        float phaseFuncResult = CalculatePhaseFunctiuon(phaseTheta);
        float angleBetweenSunlightAndRay = PI / 2 - acos(dot(-sunDirection, normalize(rayPos)));
        
        float u, v;
        u = h / (atmosphereRadius - groundRadius);
        v = (sin(angleBetweenSunlightAndRay) + 1) * 0.5;
        float3 sf = shadowFactor.Sample(smp, float2(u, v));
        // S(x,li)計算
        
        scattering += (nextT - currentT) * sigmaS * transmittanceFromRayToEye * phaseFuncResult * sf;
    }

    sumSigmaT += deltaSigmaT;
}
float4 ps_main(vsOutput input) : SV_TARGET
{
    float x = input.position.x / 255;
    float y = input.position.y / 255;
    // origin
    //float phi = input.texCoord.x * 2 * PI; // 0~2PI
    //float theta = (input.texCoord.y - 1.0f) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 
    //float phi = input.texCoord.x * 2 * PI; // 0~2PI
    //float theta = (2 * input.texCoord.y - 0.5) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 

    float phi = x * 2 * PI; // 0~2PI
    float vm = 2 * y - 1; // -1 <= vm <=1 vm>0は球の上半分、vm<0は球の下半分。teCoord.yによる0～1の範囲を広げている。画面上ピクセルの縦方向位置に対応している。
    // thetaは基本球面座標系のθ(y角)で、vmをもう一回掛けて非線形になっている。
    float theta = (y - 1.0f) * 2 * 0.5f * PI; // -PI/2 ~ PI/2 ; // PI / 2 を掛けてy軸の範囲を-PI/2～PI/2に変化させている。 sign：x < 0 ? -1: 1　vm*vmで線形→非線形化している。vm二乗による符号の消失をsign(vm)で残している。


    
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float2 twoDimDir = (cosTheta, sinTheta); // 視線方向をxy平面で定義する。空は一様であるため球面座標におけるphi(x→z)は考慮しない。
    float2 cameraPos2D;
    cameraPos2D.x = 0;
    cameraPos2D.y = eyePos.y + groundRadius;
    float endT;
    if (!DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, groundRadius, endT))
    {
        DiscriminateIntersectionWithCircle(cameraPos2D, twoDimDir, atmosphereRadius, endT);
    }
    //return float4(0.0f,1.0f,0.0f,1.0f);
    // 球面座標でレイマーチング
    float3 cameraPos3D = (0, 0, 0);
    cameraPos3D.y = eyePos.y + groundRadius;
    float3 dir = (cosTheta * cosPhi, sinTheta, cosTheta * sinPhi); // スクリーン座標を球面座標に変換する。レイ方向は解像度 / 360°確保出来る。
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
    
    return float4(scattering * sunIntensity , 1);

}
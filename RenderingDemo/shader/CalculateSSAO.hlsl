RWTexture2D<float4> ssao; // UAV�I�u�W�F�N�g
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

// TODO:�m�C�Y�e�N�X�`�����痐���쐬
float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float3 getCosHemisphereSample(float randSeed1, float randSeed2, float randSeed3, float3 hitNorm)
{
// 2�̃����_���Ȑ��l���擾
    float2 randVal = float2(randSeed1, randSeed2);
// �@���ɐ����ȃx�N�g�������i�Ō�ɗ��p����j
    //float3 bitangent = getPerpendicularVector(hitNorm);
    //float3 tangent = cross(bitangent, hitNorm);
    //hitNorm = mul(view, hitNorm);
    float3 omega = normalize(float3(randSeed1, randSeed2, randSeed3));
    float3 tangent = normalize(omega - hitNorm * dot(omega, hitNorm));
    float3 bitangent = cross(tangent, hitNorm);
// �f�B�X�N��Ɉ�l�ɃT���v�����O
    randVal.x *= sign(randVal.x);
    //randVal.y *= sign(randVal.y);
    float r = sqrt(randVal.x);
    float phi = 2.0f * PI * randVal.y;
// �����Ɏˉe����
    float x = r * cos(phi);
    float z = r * sin(phi);
    float y = sqrt(1.0 - randVal.x); // 1- r2
// �@���x�N�g���̍��W�n�Ɏˉe
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
//    uint2 pixIdx = DispatchRaysIndex().xy; // ���C�C���f�b�N�X x=0�`1920, y=0�`1080
//    uint2 numPix = DispatchRaysDimensions().xy; // �X�e�[�W�T�C�Y x=1920, y=1080
//// �����_���ȃV�[�h���v�Z
    
//    uint randSeed = initRand(pixIdx.x + pixIdx.y * numPix.x, 100);
// �Օ��x����
    float visibility = 0.0f;
// ��΂����C�̉�
    const int aoRayCount = 4;
    for (int i = 0; i < aoRayCount; ++i)
    {
// �@���𒆐S�Ƃ���������̃����_���ȃx�N�g���̃T���v�����O�i�R�T�C���d�ݕt�����z�j
        float3 sampleDir = getCosHemisphereSample(randSeed1, randSeed2, randSeed3,normal);
// �V���h�E���C���΂�
        float sampleVisibility = shootShadowRay(position, sampleDir, RAY_EPSILON, 10.0, 0.05, depth, normal);
//�Օ��x���� += �T���v�����O�����l �~ �R�T�C���� / �m�����x�֐�
        float NoL = /*saturate(*/dot(normal, sampleDir)/*)*/;
        float pdf = NoL / PI;
        visibility += sampleVisibility * NoL / pdf;
    }
// ���ς����
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
    // �����̃I�u�W�F�N�g�͑ΏۊO�Ƃ���
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
        // ���ƃL�����N�^�[�Ƃ�SSAO���v�Z���Ȃ����߂̏���1
        // ���̏����̂��߂ɃL�����N�^�[�̖@���o�͒l�͑S��0�Ƃ��Ă���
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

                // RTAO�e�X�g
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
                
                // �����̌��ʖ@���̔��Α��Ɍ����Ă����甽�]
                float dt = dot(norm, omega);
                float sgn = sign(dt);
                //omega *= sgn;
                dt *= sgn; // ���̒l�ɂ���cos�Ƃ𓾂�      
                
                // �s�N�Z����view z���傫��(����)�قǃT���v�����O���a��傫������
                radius += saturate(respos.z / 5000);
                
                // �J�[�e���ɑ΂��Ă͈ꗥ���a
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

                // �v�Z���ʂ����݂̏ꏊ�̐[�x��艜�ɓ����Ă���Ȃ�Ւf����Ă���̂ŉ��Z����
                float sampleDepth = samplePos.z;
                float depthDifference = abs(dp2 - dp);
                
                // �J�[�e���Ə��Ɉ��肵��SSAO�𐶐����邽�߂̏���
                if (respos.y <= -1.55f)
                {
                    depthDifference = 0.0f;
                }
                
                // ���ɑ΂��ăT���v�����O�����Ȃ�����
                //if (samplePos.y <= -1.55f)
                //{
                //    depthDifference = 1.0f;
                //}
                
                // ���ƃL�����N�^�[�Ƃ�SSAO���v�Z���Ȃ����߂̏���2
                if (sOriNorm.x + sOriNorm.y + sOriNorm.z == 3.0f || sOriNorm.z > 0.96f)
                {
                    depthDifference = 1.0f;
                }
                
                float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(respos.z - sampleDepth));
                if (depthDifference <= /*0.0028f*/0.015f + saturate(1.0f / respos.z)) // saturate...���Z�ɂ���ċ߂��̃r���[��Ԃɂ����ăJ�����߂��ق�臒l��傫�����Đ�������߁A�J�����ɋ߂Â��ق�SSAO�̌v�Z������Ȃ����ۂ�������Ă���
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
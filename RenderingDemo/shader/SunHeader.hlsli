#include "..\Definition.hlsli"

cbuffer BillboardMatrix : register(b0) // ���݃J�������`�悵�Ă���X�N���[�����W4���̃��[���h��Ԃɂ�����x�N�g��
{
    matrix world;
    matrix view;
    matrix proj;
    matrix cameraPos;
    matrix sunDir;
    matrix billboard;
    matrix scene;
    float3 sunTheta;
};

SamplerState smp : register(s0); // No.0 sampler
Texture2D<float3> shadowFactor : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    //float4 clipPos : CLIP_POSITION;
    float2 texCoord : TEXCOORD;
};
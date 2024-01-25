cbuffer Frustum : register(b0) // ���݃J�������`�悵�Ă���X�N���[�����W4���̃��[���h��Ԃɂ�����x�N�g��
{
    float4 topLeftFrustum;
    float4 topRightFrustum;
    float4 bottomLeftFrustum;
    float4 bottomRightFrustum;
};

cbuffer worldMatrix : register(b1) // �L�[�����ɂ��world���W�ω�
{
    matrix world;
};

SamplerState smp : register(s0);
Texture2D<float4> SkyLUT : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 topLeft : TOPLEFT;
    float3 topRight : TOPRIGHT;
    float3 bottomLeft : BOTTOMLEFT;
    float3 bottomRight : BOTTOMRIGHT;
};
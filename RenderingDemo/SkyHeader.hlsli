cbuffer Frustum : register(b0) // ���݃J�������`�悵�Ă���X�N���[�����W4���̃��[���h��Ԃɂ�����x�N�g��
{
    float4 topLeft;
    float4 topRight;
    float4 bottomLeft;
    float4 bottomRight;
};

SamplerState smp : register(s0);
Texture2D<float4> SkyLUT : register(t0);

struct vsOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};
Texture2D<float4> tex : register(t0); // sponza�`��
Texture2D<float4> tex2 : register(t1); // �L�����N�^�[�`��
SamplerState smp : register(s0); // �T���v���[

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

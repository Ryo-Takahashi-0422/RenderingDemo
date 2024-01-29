Texture2D<float4> tex : register(t0); // sponza�`��
Texture2D<float4> tex2 : register(t1); // �L�����N�^�[�`��
Texture2D<float4> sponzaDepthmap : register(t2); // sponza�f�v�X�}�b�v
Texture2D<float4> connanDepthmap : register(t3); // �L�����N�^�[�f�v�X�}�b�v
Texture2D<float4> sky : register(t4); // skyLUT
Texture2D<float4> imguiWindow : register(t5); // skyLUT

SamplerState smp : register(s0); // �T���v���[

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

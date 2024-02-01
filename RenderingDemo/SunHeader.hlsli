cbuffer BillboardMatrix : register(b0) // ���݃J�������`�悵�Ă���X�N���[�����W4���̃��[���h��Ԃɂ�����x�N�g��
{
    matrix world;
    matrix view;
    matrix proj;
    matrix cameraPos;
    matrix sunDir;
    matrix billboard;
};

struct vsOutput
{
    float4 position : SV_POSITION;
    float4 clipPos : CLIP_POSITION;
    float2 texCoord : TEXCOORD;
};
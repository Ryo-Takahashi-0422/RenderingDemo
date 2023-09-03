struct Output
{
    float4 svpos : SV_POSITION; // �V�X�e���p���_���W
    float4 pos : POSITON; // ���_���W
    float4 norm : NORMAL0; // �@���x�N�g��
    //float4 vnormal : NORMAL1; // �r���[�ϊ���̖@���x�N�g��
    float2 uv : TEXCOORD; // uv�l
    //float3 ray : VECTOR; // ���_�x�N�g��
    //uint instNo : SV_InstanceID; // DrawIndexedInstanced��instance id
    //float4 tpos : TPOS;
};

struct PixelOutput
{
    float4 col : SV_TARGET0; // model color rendering
    //float4 mnormal : SV_TARGET1; // model normal rendering
    //float4 highLum : SV_TARGET2; // model high luminance rendering
};

cbuffer SceneBuffer : register(b0) // �ϊ��s��
{
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    //matrix lightCamera; // view matrix from light * orthographic projection matrix
    //matrix shadow; // shadow matrix
    //float3 eye; // eye(camera) position
    //matrix invProj; // inverse matrix of projection matrix
    //matrix invView; // inverted view matrix 
    //matrix bones[256]; // bone matrix

    //float3 lightVec;
    //bool isSelfShadow;
};

cbuffer Material : register(b1)
{
    float3 diffuse;
    float3 ambient;
    float3 emissive;
    float3 bump;
    float3 specular;
    float3 reflection;
    float shinenes;
}

SamplerState smp : register(s0); // No.0 sampler
//SamplerState smpToon : register(s1); // No.1 sampler(toon)
//SamplerComparisonState smpBilinear : register(s2); // No.2 sampler
//
Texture2D<float4> colormap : register(t0);
Texture2D<float4> normalmap : register(t1);
Texture2D<float4> specularmap : register(t2);
Texture2D<float4> metalmap : register(t3);
Texture2D<float> transparentmap : register(t4);
//Texture2D<float4> toon : register(t5); //No.5 toon texture
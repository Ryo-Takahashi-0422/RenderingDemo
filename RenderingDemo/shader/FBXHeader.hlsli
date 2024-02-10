struct Output
{
    float4 svpos : SV_POSITION; // システム用頂点座標
    float4 pos : POSITON; // 頂点座標
    float4 norm : NORMAL0; // 法線ベクトル
    //float4 vnormal : NORMAL1; // ビュー変換後の法線ベクトル
    float2 uv : TEXCOORD; // uv値
    float4 lightTangentDirection : LightTangentDirection;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float3 normal : NORMAL2;

    //float3 ray : VECTOR; // 視点ベクトル
    //uint instNo : SV_InstanceID; // DrawIndexedInstancedのinstance id
    //float4 tpos : TPOS;
};

struct PixelOutput
{
    float4 col : SV_TARGET0; // model color rendering
    //float4 mnormal : SV_TARGET1; // model normal rendering
    //float4 highLum : SV_TARGET2; // model high luminance rendering
};

cbuffer SceneBuffer : register(b0) // 変換行列
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
    //matrix ReverceMatrixOfInitialPosture[256]; // index number is equal with bones index number
    matrix bones[256]; // pmd bone matrix // index number is equal with bones index number
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
Texture3D<float4> airmap : register(t0);
Texture2D<float4> colormap : register(t1);
Texture2D<float4> normalmap : register(t2);
Texture2D<float4> specularmap : register(t3);
Texture2D<float4> metalmap : register(t4);
Texture2D<float> transparentmap : register(t5);
//Texture2D<float4> toon : register(t5); //No.5 toon texture
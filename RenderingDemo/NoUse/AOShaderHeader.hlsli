Texture2D<float> depthmap : register(t0); // No.0 depthmap
Texture2D<float4> normalmap : register(t1); // No.1 lightmap(depth from light-view)


cbuffer SceneBuffer : register(b0) // affine transformation matrix
{
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    matrix lightCamera; // view matrix from light * orthographic projection matrix
    matrix shadow; // shadow matrix
    float3 eye; // eye(camera) position
    matrix invProj; // inverse matrix of projection matrix
    matrix invView; // inverted view matrix 
    matrix bones[256]; // bone matrix
    
    float3 lightVec;
    bool isSelfShadow;
};

SamplerState smp : register(s0); // sampler
SamplerState smp2 : register(s1); // sampler

struct AoOutput
{
    float ao : SV_TARGET0; // model color rendering
};

struct Output
{
    float4 svpos : SV_POSITION; // システム用頂点座標
    float4 pos : POSITON; // 頂点座標
    float4 norm : NORMAL0; // 法線ベクトル
    float4 vnormal : NORMAL1; // ビュー変換後の法線ベクトル
    float2 uv : TEXCOORD; // uv値
    float3 ray : VECTOR; // 視点ベクトル
    uint instNo : SV_InstanceID; // DrawIndexedInstancedのinstance id
    float4 tpos : TPOS;
};

float random(float2 uv);
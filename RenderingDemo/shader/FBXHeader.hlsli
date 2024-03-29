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
    
    float4 screenPosition : SCREEN_POSITIO;
    float4 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float4 lvPos : LIGHTVIEW_POSITION;
    bool isEnhanceShadow : ENHANCESHADOW;
    bool isChara : chara;
    float3 truePos : TRUE_POSITION;
    float lvDepth : LIGHTVIEW_DEPTH;
    float trueDepth : TRUE_DEPTH;
    float3 light : LIGHT;
    float adjust : ADJUST;
    float index : INDEX;
    float4 rotatedNorm : ROTATED_NORMAL;
    float3 ray : RAY;
    //float3 ray : VECTOR; // 視点ベクトル
    //uint instNo : SV_InstanceID; // DrawIndexedInstancedのinstance id
    //float4 tpos : TPOS;
};

struct PixelOutput
{
    float4 col : SV_TARGET0; // model color rendering
    float4 normal : SV_TARGET1; // model normal rendering
    //float4 highLum : SV_TARGET2; // model high luminance rendering
};

cbuffer SceneBuffer : register(b0) // 変換行列
{
    matrix charaRot; // キャラクターの回転行列を転置したもの。
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    matrix oProj;
    matrix bones[256]; // pmd bone matrix // index number is equal with bones index number
    
    matrix rotation;
    matrix shadowPosMatrix;
    matrix shadowPosInvMatrix;
    matrix shadowView;
    float3 eyePos;
    float3 sunDIr;
    float3 charaPos;
    bool sponza;
    bool airDraw;
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
Texture2D<float4> vsmmap : register(t1);
Texture2D<float4> depthmap : register(t2);
Texture2D<float4> colormap : register(t3);
Texture2D<float4> normalmap : register(t4);
Texture2D<float4> specularmap : register(t5);
Texture2D<float4> metalmap : register(t6);
Texture2D<float> transparentmap : register(t7);
//Texture2D<float4> toon : register(t5); //No.5 toon texture
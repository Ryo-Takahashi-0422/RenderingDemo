struct PixelOutput
{
    float4 col : SV_TARGET0; // model color rendering
    float4 normal : SV_TARGET1; // model normal rendering
};

cbuffer SceneBuffer : register(b0) // 変換行列
{
    matrix charaRot; // キャラクターの回転行列を転置したもの。
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    matrix oProj;
    matrix bones[256]; // pmd bone matrix // index number is equal with bones index number
    //matrix invProj;
    matrix rotation;
    matrix shadowPosMatrix;
    matrix shadowPosInvMatrix;
    matrix shadowView;
    float3 eyePos;
    float3 sunDIr;
    float3 charaPos;
    bool sponza;
    bool airDraw;
    bool brdfDraw;
    float3 plPos1;
    float3 plPos2;
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

Texture3D<float4> airmap : register(t0);
Texture2D<float4> vsmmap : register(t1);
Texture2D<float> depthmap : register(t2); // occ
Texture2D<float4> colormap : register(t3);
Texture2D<float4> normalmap : register(t4);
Texture2D<float4> specularmap : register(t5);
Texture2D<float4> metalmap : register(t6);
Texture2D<float> transparentmap : register(t7);
//Texture2D<float4> toon : register(t5); //No.5 toon texture

// BRDF スペキュラはhttps://zenn.dev/mebiusbox/books/619c81d2fbeafd/viewer/7c1069コードを自環境に修正して算出
#define PI 3.14/*159265359*/
#define PI2 6.28/*318530718*/
#define EPSILON 1e-3/*6*/
#define LIGHT_MAX 2
#define OCC_BIAS 0.008

struct IncidentLight
{
    float3 color;
    float3 direction;
    //bool visible;
};

struct ReflectedLight
{
    //float3 directDiffuse;
    float3 directSpecular;
};

struct GeometricContext
{
    float3 position;
    float3 normal;
    float3 viewDir;
};

struct Material
{
    //float3 diffuseColor;
    float specularRoughness;
    //float3 specularColor;
};

struct PointLight
{
    float3 position;
    //float3 color;
    float distance;
    //float decay;
};

struct DirectionalLight
{
    float3 direction;
    float3 color;
};

struct Output
{
    float4 svpos : SV_POSITION; // システム用頂点座標
    float4 pos : POSITON; // 頂点座標

    float2 uv : TEXCOORD; // uv値
    
    float4 screenPosition : SCREEN_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float4 lvPos : LIGHTVIEW_POSITION;

    bool isChara : chara;

    float trueDepth : TRUE_DEPTH;
    float3 light : LIGHT;
    float adjust : ADJUST;

    float4 viewPos : VIEWPOSITION;
    float4 viewSpacePos : VIEWSPACEPOS;
    float4 wPos : WPOS;
    float3 oriWorldNorm : ORI_WORLD_NORMAL;
    
    float3 vEyeDirection : NORMAL_ED;
    float3 vLightDirection : NORAMAL_LD;
    float3 sLightDirection : SPONZA_SPECULAR_LD;
    
    DirectionalLight directionalLight : D_LIGHT;
    PointLight pointLights[LIGHT_MAX] : P_LIGHT;
    float weaken : WEAK_PARAM;
    
    bool isEnhanceShadow : ENHANCESHADOW;
};

Texture2D<float4> tex : register(t0); // multipass1 texture(background texture)
Texture2D<float4> model : register(t1); // multipass2 texture(pmd model rendering texture)
Texture2D<float4> normalmap : register(t2); // ノーマルマップ
Texture2D<float> depthmap : register(t3); // デプスマップ
Texture2D<float> lightmap : register(t4); // ライトマップ
Texture2D<float4> multinormalmap : register(t5); // マルチターゲットにレンダリングした法線マップ
Texture2D<float4> bloommap : register(t6); // bloom
Texture2D<float4> shrinkedbloommap : register(t7); // shrinked bloom map
Texture2D<float4> shrinkedModel : register(t8); // shrinked bloom map
Texture2D<float> aomap : register(t9); // shrinked bloom map
Texture2D<float4> imgui : register(t10); // shrinked bloom map

SamplerState smp : register(s0); // sampler

cbuffer PostEffect : register(b0) // post-effect vector
{
    float4 bkweights[2];
};

cbuffer SceneBuffer : register(b1) // affine transformation matrix
{
    matrix world; // world matrix
    matrix view; // view matrix
    matrix proj; // projection matrix
    matrix lightCamera; // view matrix from light * orthographic projection matrix
    matrix shadow; // shadow matrix
    float3 eye; // eye(camera) position
    matrix invProj; // inverse matrix of projection matrix
    matrix bones[256]; // bone matrix
    
    float3 lightVec;
    bool isSelfShadow;
};

cbuffer PostSetting : register(b2) // imgui PostSetting
{
    bool isFoV;
    float3 bloomCol;
    bool isSSAO;
    float dummy; // to alignment
    bool isBloom;
};

struct Output
{
    float4 svpos : SV_POSITION;
    float4 norm : NORMAL0;
    float2 uv : TEXCOORD;
    float4 tpos : TPOS;
};

struct BlurOutput
{
    float4 highLum : SV_TARGET0; // high luminance texture
    float4 blurModel : SV_TARGET1; // blured model texture
};

// return grayscale of PAL(phase alternating line、位相反転線) from input
float4 MakePAL(float4 col);

// return inversion color value
float4 RevrseColor(float4 col);

// down color gradation
float4 DownColorGradation(float4 col);

// emboss
float4 Emboss(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy);

// sharpness
float4 Sharpness(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy);

// return gaussian blur matrix
float4 Get5x5GaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv, float dx, float dy);

float4 Gauss(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect);

// blur by pixel averaging
float4 AverageBlur(Texture2D _texture, SamplerState _smp, float2 _uv, int offset, float dx, float dy);

// simple gaussian blur
float4 SimpleGaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv/*, float dx, float dy*/);

// normalize texture use
float4 NormalmapEffect(float2 _uv);

// deffered shading
float4 DefferedShading(float2 _uv);

// shrinkedbloommap use case(bloom effect)
float4 BloomEffect(Texture2D _texture, float2 _uv);

// Field Of Depth
float4 FOVEffect(Texture2D _texture, SamplerState _smp, float2 _uv, float focusDistance);

float SSAOBlur(SamplerState _smp, float2 _uv);
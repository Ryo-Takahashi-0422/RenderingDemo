#define FXAA_PC 1
#define FXAA_HLSL_5 1
//#define FXAA_QUALITY__PRESET 12
#include "FXAA.hlsl"
#include "PeraHeader.hlsli"

#define    PIXEL_SIZE g_aTemp[0].xy
#define    SUBPIX g_aTemp[1].x
#define    EDGE_THRESHOLD g_aTemp[1].y
#define    EDGE_THRESHOLD_MIN g_aTemp[1].z

cbuffer Infos : register(b0) // from PreFxaa
{
    float size;
    bool isFxaa;
};

float4 ps_main(Output input) : SV_TARGET
{
    float2 uv = input.uv;
    FxaaTex InputFXAATex = { smp, colorTex };
    return FxaaPixelShader(
        uv, // FxaaFloat2 pos,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsolePosPos,
        InputFXAATex, // FxaaTex tex,
        InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegOne,
        InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        float2(1.0f / 1000.0f, 1.0f / 1000.0f), // FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.25, // FxaaFloat fxaaQualitySubpix,
        0.154, // FxaaFloat fxaaQualityEdgeThreshold,
        0.0358, // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f) // FxaaFloat fxaaConsole360ConstDir,
    );
}
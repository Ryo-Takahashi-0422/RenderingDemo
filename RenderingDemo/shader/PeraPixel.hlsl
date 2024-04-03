#include "PeraHeader.hlsli"

float4 ps(Output input) : SV_TARGET
{   
    float4 ret = (0, 0, 0, 1);
    float w, h, levels;
    colorTex.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    
    float focus = 0.0f; // focus distance
    float dp = depthMap.Sample(smp, input.uv);
    dp = pow(dp, 20.0); // Calculation between small values (y-dp) isn't effective. 'dp' should be emphasized enough before (y-dp).
    
    float depthDiff = abs(focus - dp);
    depthDiff = pow(depthDiff, 2.0f);
    
    float4 retColor[2];
    retColor[0] = colorTex.Sample(smp, input.uv);
    retColor[1] = bluredColor.Sample(smp, input.uv);;
    
    depthDiff = saturate(depthDiff * 2);
    ret = lerp(retColor[0], retColor[1], depthDiff);
    
    float4 imgui = imguiTex.Sample(smp, input.uv);
    float ssao = ssaoTex.Sample(smp, input.uv);
    
    float imguiTotal = 0.0f;
    imguiTotal = imgui.x + imgui.y + imgui.z;
    if (imguiTotal != 0)
    {
        return imgui * 2.0f;
    }
    
    //ret += imgui;
    ret = lerp(ret * 0.3f, ret, ssao);
    return ret;
}
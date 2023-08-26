#include "BasicShaderHeader.hlsli"

float4 LightMapVS
(float4 pos : POSITION,
float4 norm : NORMAL,
float2 uv : TEXCOORD,
min16uint2 boneno : BONE_NO,
min16uint weight : WEIGHT): SV_POSITION
{
    float w = float(weight) / 100.0f;
    matrix bm = bones[boneno[0]] * w + bones[boneno[1]] * (1 - w);
    pos = mul(bm, pos);
    
    //pos = mul(mul(mul(proj, view), world), pos) /*mul(lightCamera, pos)*/;
    //return pos;
    return mul(lightCamera, pos);
}
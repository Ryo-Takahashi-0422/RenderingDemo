#include "FBXHeaderShader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float3 norm : NORMAL,
    float2 uv : TEXCOORD
    //min16uint2 boneno : BONE_NO,
    //min16uint weight : WEIGHT,
    //uint instNo : SV_InstanceID
)
{
    Output output; // ピクセルシェーダーに渡す値
    //float w = weight / 100.0f;
    //matrix bm = bones[boneno[0]] * w + bones[boneno[1]] * (1 - w);
    //pos = mul(bm, pos);

    //if(instNo == 1)
    //{
    //    pos = mul(shadow, pos); // 影を計算
    //}

    // pixelshaderへの出力を作っていく
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
    //norm.w = 0; // worldに平行移動成分が含まれている場合、法線が並行移動する。(この時モデルは暗くなる。なぜ？？)
    output.norm = mul(world, norm);
    //output.vnormal = mul(view, output.norm);
    output.uv = uv;
    //output.ray = normalize(pos.xyz - eye);
    //output.instNo = instNo;
    //output.tpos = mul(lightCamera, pos); // world乗算をしても結果が変わらないのは、使っているworldが単位行列だから

    return output;
}
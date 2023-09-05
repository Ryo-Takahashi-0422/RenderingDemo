#include "FBXHeaderShader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive
)
{
    Output output; // ピクセルシェーダーに渡す値
    //float w = weight / 100.0f;
    //matrix bm = bones[boneno[0]] * w + bones[boneno[1]] * (1 - w);
    //pos = mul(bm, pos);
    
    //matrix bm1 = bones[boneno1[0]] * (1 - boneweight1[boneno1[0]]); // boneweight1[boneno1[0]]以外の全てのウェイトでは立方体が元のアニメーション通りに動く
    //matrix bm2 = bones[boneno1[1]] * (1 - boneweight1[boneno1[1]]);
    //matrix bm3 = bones[boneno1[2]] * (1 - boneweight1[boneno1[2]]);
    
    //matrix bm4 = bones[boneno2[0]] * (1 - boneweight2[boneno2[0]]);
    //matrix bm5 = bones[boneno2[1]] * (1 - boneweight2[boneno2[1]]);
    //matrix bm6 = bones[boneno2[2]] * (1 - boneweight2[boneno2[2]]);
    
    matrix bm1 = mul(bones[boneno1[0]], ReverceMatrixOfInitialPosture[boneno1[0]]) * (1 - boneweight1[boneno1[0]]);
    matrix bm2 = mul(bones[boneno1[1]], ReverceMatrixOfInitialPosture[boneno1[1]]) * (1 - boneweight1[boneno1[1]]);
    matrix bm3 = mul(bones[boneno1[2]], ReverceMatrixOfInitialPosture[boneno1[2]]) * (1 - boneweight1[boneno1[2]]);
    
    matrix bm4 = mul(bones[boneno2[0]], ReverceMatrixOfInitialPosture[boneno2[0]]) * (1 - boneweight2[boneno2[0]]);
    matrix bm5 = mul(bones[boneno2[1]], ReverceMatrixOfInitialPosture[boneno2[1]]) * (1 - boneweight2[boneno2[1]]);
    matrix bm6 = mul(bones[boneno2[2]], ReverceMatrixOfInitialPosture[boneno2[2]]) * (1 - boneweight2[boneno2[2]]);
    
    matrix bm = bm1 + bm2 + bm3 + bm4 + bm5 + bm6;
    pos = mul(bm, pos);
    
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
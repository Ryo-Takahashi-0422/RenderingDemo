#include "Shadow.hlsli"

vsOutput vs_main
(
    float4 pos : POSITION,
    float4 norm : NORMAL,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
uint index : SV_VertexID
)
{
    vsOutput output;
    output.adjust = 200.0f;
    output.specialObj = false;
    if (boneweight1[0] == 0 /*&& boneweight1[1] == 0 && boneweight1[2] == 0 && boneweight2[0] == 0 && boneweight2[1] == 0 */&& boneweight2[2] == 0)
    {
        output.position = mul(mul(proj, view), pos);
        
        // vsmの出力結果を安定させる。この処理がないとrgb値にムラが出る。利用する側でも同様の処理を行う必要あり。
        float3 oLightPos = lightPos;
        
        float3 worldPos = pos;
        output.worldPos = pos;
        output.trueDepth = length(worldPos - lightPos) / output.adjust;
        
        // 処理軽減のためコメントアウト
        // ポール影がキャラクター背面に貫通するのが目立つので、対策。キャラクターのレンダリング時にポールの落ち影かどうか判定するのに利用する。
        //if ((77496 <= index && index <= 91841))
        //{
        //    output.specialObj = true;
        //}

        output.index = index;
        
        return output;
    }
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm = bm1 + bm2 + bm3/* + bm4 + bm5 + bm6*/;   
    
    pos = mul(bm, pos);
    pos = mul(rotation, pos);

    output.position = mul(mul(mul(proj, view), world), pos);
    
    float3 oLightPos = lightPos;
    
    float3 worldPos = mul(world, pos);
    output.worldPos = worldPos;
    output.trueDepth = length(worldPos - oLightPos) / output.adjust;

    return output;
}
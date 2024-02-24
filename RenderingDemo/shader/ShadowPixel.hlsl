#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    
    //float depth = length(input.worldPos - lightPos) / 100;
    //float depth2 = depth * depth;
    
    // [0],[1]:キャラクターのみ光源を近づけた場合の値　キャラクターの影を地面に濃く投影するためのvsm
    // [2]:全て通常の光源位置での距離　[0],[1]をキャラクターのセルフシャドウ描画で利用するとdepthがvsm値より大きくなり常に影になってしまう。
    return float4( /*0.0f*/input.depthAndLength.x/*depth*/, /*0.0f*/input.depthAndLength.y /*depth2*/, input.trueDepth, 1.0f);
}
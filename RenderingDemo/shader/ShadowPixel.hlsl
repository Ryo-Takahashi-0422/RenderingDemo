#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    float3 oLightPos = lightPos;
    // 2024/2/28時点、sponzaの草花や花瓶など高度低めのオブジェクトの影が薄くなる問題に対する手段は以下以外にない...以下のようにあらゆるオブジェクトにライトを近づけて影を強調すると、太陽高度低めのときに外壁が光を貫通してしまう...
    // pixより拾った頂点インデックス番号で影を強調するオブジェクトを直に指定している。ちなみにキャラクター描画時にはインデックスが0開始且つ25714以下に収まるため、キャラクターの影も強調出来ている。現状はもうこれ以外に思いつかない...
    // 0...25714 
    // 49897...77462 
    // 77496...91841 壁ポール
    // 229613...233693 
    if (input.index <= 25714 || (49897 <= input.index && input.index <= 77462) || (77496 <= input.index && input.index <= 91841) || (229613 <= input.index && input.index <= 233693))
    {
        float newY = input.worldPos.y + 5.0f;
        float div = oLightPos.y / newY;
        oLightPos /= div;
    }
    
    float depth = length(input.worldPos - oLightPos) / input.adjust;
    float depth2 = depth * depth;
    
    // [0],[1]:キャラクターのみ光源を近づけた場合の値　キャラクターの影を地面に濃く投影するためのvsm
    // [2]:全て通常の光源位置での距離　[0],[1]をキャラクターのセルフシャドウ描画で利用するとdepthがvsm値より大きくなり常に影になってしまう。
    return float4( /*0.0f*/ /*input.depthAndLength.x*/depth, /*0.0f*/ /*input.depthAndLength.y*/depth2, input.trueDepth, input.specialObj);
}
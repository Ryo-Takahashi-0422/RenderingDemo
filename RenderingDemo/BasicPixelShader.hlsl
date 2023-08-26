#include "BasicShaderHeader.hlsli"

/*float4*/PixelOutput BasicPS(Output input) : SV_TARGET
{    
    PixelOutput output;
    //if (input.instNo == 1)
    //{
    //    return float4(0, 0, 0, 1);
    //}
    
    float3 light = normalize( /*float3(1, -1, 1)*/lightVec);
    float3 lightColor = float3(1, 1, 1);
    
    // ディフューズ計算
    float diffuseB = saturate(dot(-light, input.norm.xyz));
    float4 toonDif = toon.Sample(smpToon, float2(0, diffuseB));
    
    //光の反射ベクトル
    float3 refLight = normalize(reflect(light, input.norm.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
    float4 toonSpecB = toon.Sample(smpToon, float2(0, specularB));
    
    //スフィアマップ用UV
    float2 sphereMapUV = (input.vnormal.xy + float2(1, -1)) * float2(0.5, -0.5);
    
    // テクスチャカラー
    float4 texColor = tex.Sample(smp, input.uv);
      
    float4 result = saturate(diffuse * texColor + toonSpecB * float4(specular.rgb, 1));
   
    //return result;
    
    //float4 result = max(saturate(
    //toonDif
    //* diffuse
    //* texColor
    //* sph.Sample(smp, sphereMapUV))
    //+ saturate(spa.Sample(smp, sphereMapUV) * texColor
    //+ toonSpecB * float4(specular.rgb, 1)), float4(texColor.xyz * ambient, 1));
    //return result;
    
    
    
    
    light = normalize( /*float3(1, -1, 1)*/lightVec);
    float bright = dot(input.norm, -light);
    
    float shadowWeight = 1.0f;
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w; // -1<=x<=1, -1<=y<=1, 0<=z<=1
    // 0<=x<=1, -1<=y<=0に移動したい。深度マップはuv座標なので左上が(0,0)で右下が(1,1)、平行投影した空間は左上が(-1,1)で右下が(1,-1)になる。
    // 空間のxyをuvに合わせる必要がある。最初に(1,-1)を足して左上を合わせると右下が(2,-2)になる。次に右下を合わせるため(0.5,-0.5)をかけることで
    // 右下が(1,1)になり、uvの座標と合致する。これをuv座標として利用する。
    float2 shadowUV = (posFromLightVP.xy + float2(1, -1)) * float2(0.5, -0.5);
    //float depthFromLight = lightmap.Sample(smp, shadowUV);
    
    //if (depthFromLight < posFromLightVP.z - 0.001f)
    //{
    //    shadowWeight = 0.5f;
    //}
    //shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    
    // lightmapをlightmapUVでサンプリングした結果と、今処理している点のlightビューに変換した時のz(深度)座標を比較している
    // 例：sample結果が0(深度マップ値、黒)で、光源目線のビュー変換&射影変換したときの今の点のz座標が0.15なら、今の点は影になる
    // なので、shadowWeightを0.5として結果に掛けて、色を暗く変化させている
    float depthFromLight = lightmap.SampleCmp(smpBilinear, shadowUV, posFromLightVP.z - 0.005f);

    shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    float b = /*bright **/ shadowWeight;
        
    float lmap = pow(lightmap.Sample(smp, shadowUV), 0.3); // このuvはモデルのuvである。レンダリングされた画像のuvとは違うので上手く読めない？
    float4 lmap4 = float4(lmap, lmap, lmap, 1);
    //return lmap4; //!!!この結果がBufferPixelと異なっている。こちらがオカシイ。テクスチャが読み込めていない様子。
    
    if(isSelfShadow)
    {
        output.col = shadowWeight * result;
    }
    else
    {
        output.col = result;
    }
        output.mnormal.rgb = float3((input.norm.xyz + 1.0f) / 2.0f);
        float y = dot(float3(0.299f, 0.587f, 0.114f), output.col.xyz);
        output.highLum = y > 0.7f ? y : 0.0f;
        return output;
    
    }
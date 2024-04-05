#include "FBXHeader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL_Vertex,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
    float3 tangent : TANGENT,
    float3 binormal : BINORMAL,
    float3 vnormal : NORMAL,
uint index : SV_VertexID)
{
    Output output; // ピクセルシェーダーに渡す値
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm4 = bones[boneno2[0]] * boneweight2[0];
    matrix bm5 = bones[boneno2[1]] * boneweight2[1];
    matrix bm6 = bones[boneno2[2]] * boneweight2[2];
    
    matrix bm = bm1 + bm2 + bm3 + bm4 + bm5 + bm6;
    bool moveObj = true;
    
    output.adjust = 200.0f;
    float3 lightPos;
    float adjustDirValue = 95.0f;
    lightPos.x = adjustDirValue * sunDIr.x;
    lightPos.y = -adjustDirValue * sunDIr.y;
    lightPos.z = adjustDirValue * sunDIr.z;
    output.light = lightPos;
    
    // 壁の法線値が-1になりssaoが計算出来くなる問題への対処
    float4 m_normal = norm;
    float4 oriNorm = norm;
    
    m_normal = normalize(m_normal);
 
    matrix rot = charaRot;
    output.rotatedNorm = m_normal;
    output.rotatedNorm = mul(rot, output.rotatedNorm);
    
    if (boneweight1[0] == 0 && boneweight2[2] == 0 && sponza)
    {
        m_normal.x *= sign(m_normal.x);
        m_normal.z *= sign(m_normal.z);
        
        moveObj = false;
        //bm[0][0] = 1;
        //bm[1][1] = 1;
        //bm[2][2] = 1;
        //bm[3][3] = 1;
        output.worldPosition = pos;
        
        output.lvPos = mul(mul(oProj, shadowView), output.worldPosition);
        
        // 平行投影に合わせてライトの位置を頂点の位置の真上に移動した状態で、ライト→頂点への距離を算出している。
        //float3 lightPos = -65 * sunDIr;
        //lightPos.x += pos.x/* * lightPos.y / 65.0f*/;
        //lightPos.z += pos.z/* * lightPos.y / 65.0f*/;
        output.truePos = output.lvPos;
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //float jj = output.worldPosition.y + 5.0f;
        //float k = output.worldPosition.y / jj;
        //lightPos /= k;
        output.isEnhanceShadow = false;
        if (index <= 25714 || (49897 <= index && index <= 77462) || (77496 <= index && index <= 91841) || (229613 <= index && index <= 233693))
        {
            float newY = output.worldPosition.y + 5.0f;
            float div = lightPos.y / newY;
            lightPos /= div;
            output.isEnhanceShadow = true;
        }
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65.01f / (lightPos.y + 0.01f);
        output.isChara = false;

        output.normal = normalize(mul(world, oriNorm));
    }
    else
    {
        pos = mul(bm, pos);
        pos = mul(rotation, pos);
        output.worldPosition = mul(shadowPosMatrix, pos);
        
        output.lvPos = mul(mul(mul(oProj, shadowView), shadowPosMatrix), pos);
        
        // 平行投影に合わせてライトの位置を頂点の位置の真上に移動した状態で、ライト→頂点への距離を算出している。
        //float3 lightPos = -65 * sunDIr;
        //lightPos.x += pos.x/* * lightPos.y / 65.0f*/;
        //lightPos.z += pos.z/* * lightPos.y / 65.0f*/;

        // キャラクターのシャドウマップ上の描画は2通りある。通常のライト位置および高さを1/n倍したもので、後者はsponzaの影描画で利用する。前者はキャラクターの影描画で利用する。
        output.truePos = output.lvPos;        
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.trueDepth *= 65.01f / (lightPos.y + 0.01f);
        
        float jj = output.worldPosition.y + 5.0f;
        float k = output.worldPosition.y / jj;
        lightPos /= k;
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65 / (lightPos.y + 0.01f);
        output.isEnhanceShadow = true;
        output.isChara = true;
        
        float3x3 bmTan;
        bmTan[0] = bm[0];
        bmTan[1] = bm[1];
        bmTan[2] = bm[2];
        output.normal = normalize(float4(mul(bmTan, output.rotatedNorm.xyz), 1));
    }
    //pos = mul(bm, pos);
    //pos = mul(rotation, pos);
    
    // 床とキャラクターとのSSAOを無理矢理消す処理。弊害として、床にあらゆるSSAOが発生しなくなる。
    //if (m_normal.y == 1.0f)
    //{
    //    m_normal.y = 0.0f;

    //}


    float3 lightDirection = -sunDIr;
    output.lightTangentDirection = float4(normalize(lightDirection), 1);
     

    //output.tangent = normalize(mul(world, mul(bmTan, tangent)));
    //output.biNormal = normalize(mul(world, mul(bmTan, binormal)));

    //output.normal = normalize(mul(world, /*mul(bmTan, */m_normal.xyz)) /*)*/; connanいい感じ
    
    output.tangent = normalize(cross(output.normal, float3(0.0, 1.0, 0.0)));
    //output.tangent = normalize(mul(bmTan, output.tangent));
    output.biNormal = cross(output.normal, output.tangent);
    //output.biNormal = normalize(mul(bmTan, output.biNormal));
    float3 tEye = (float4(eyePos, 1) - mul(world, pos)).xyz;
    float3 tLight = (float4(lightPos, 1) - mul(world, pos)).xyz;
    output.vEyeDirection.x = abs(dot(output.tangent, tEye));
    output.vEyeDirection.y = dot(output.biNormal, tEye);
    output.vEyeDirection.z = abs(dot(output.normal, tEye));
    normalize(output.vEyeDirection);
    output.vLightDirection.x = dot(output.tangent, tLight);
    output.vLightDirection.y = dot(output.biNormal, tLight);
    output.vLightDirection.z = dot(output.normal, tLight);
    normalize(output.vLightDirection);
    
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
    //norm.w = 0; // worldに平行移動成分が含まれている場合、法線が並行移動する。(この時モデルは暗くなる。なぜ？？)
    output.norm = mul(world, m_normal);
    


    // タイリング対応しているuvについての処理は避ける。例えば負を正に変換する処理をすることで、テクスチャが斜めにゆがむ
    output.uv = uv;
    
    output.screenPosition = output.svpos;
    //output.worldPosition = pos; //mul(shadowPosMatrix, pos); // worldは単位行列なので乗算しない
    output.worldNormal = normalize(mul(world, m_normal).xyz) /*normalize(mul(mul(mul(proj, view), world), norm).xyz)*/;
    
    output.ray = normalize(pos.xyz - eyePos);

    //float3 rotEyepos = mul(rotation, eyePos);
    //float3 rotPos = mul(rotation, pos);
    //output.viewPos = mul(mul(view, world), rotEyepos).xyz - mul(mul(view, world), rotPos).xyz;
    //output.viewPos = mul(-rotation, mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz);
    
    float4 eye = float4(eyePos, 1);
    float4 vEye = mul(view, eye);
    
    output.wPos = mul(world, pos);
    
    float4 rotPos = mul(rotation, pos);
    float4 vPos = mul(mul(view, world), pos);
    output.viewPos = /*-eye */eye - output.wPos;
    
    output.oriWorldNorm = normalize(mul(world, oriNorm).xyz);
    //output.viewPos.z *= sign(output.viewPos.z);
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    return output;
}
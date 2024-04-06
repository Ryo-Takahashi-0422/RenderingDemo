#include "FBXHeader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL_Vertex,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
    //float3 tangent : TANGENT,
    //float3 binormal : BINORMAL,
    //float3 vnormal : NORMAL,
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
    //bool moveObj = true;
    
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
    
    float3 t_normal;
    if (boneweight1[0] == 0 && boneweight2[2] == 0 && sponza)
    {
        m_normal.x *= sign(m_normal.x);
        m_normal.z *= sign(m_normal.z);
        
        //moveObj = false;
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
        //output.truePos = output.lvPos;
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //float jj = output.worldPosition.y + 5.0f;
        //float k = output.worldPosition.y / jj;
        //lightPos /= k;
        output.isEnhanceShadow = false;
        //if (index <= 25714 || (49897 <= index && index <= 77462) || (77496 <= index && index <= 91841) || (229613 <= index && index <= 233693))
        //{
        //    float newY = output.worldPosition.y + 5.0f;
        //    float div = lightPos.y / newY;
        //    lightPos /= div;
        //    output.isEnhanceShadow = true;
        //}
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65.01f / (lightPos.y + 0.01f);
        output.isChara = false;

        t_normal = normalize(mul(world, m_normal));
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
        //output.truePos = output.lvPos;        
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.trueDepth *= 65.01f / (lightPos.y + 0.01f);
        
        float fixY = output.worldPosition.y + 5.0f;
        float enhance = output.worldPosition.y / fixY;
        lightPos /= enhance;
        output.lvDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        //output.lvDepth *= 65 / (lightPos.y + 0.01f);
        output.isEnhanceShadow = true;
        output.isChara = true;
        
        matrix bmTan;
        bmTan[0] = bm[0];
        bmTan[1] = bm[1];
        bmTan[2] = bm[2];
        bmTan[3] = float4(0,0,0,1);
        //bmTan = mul(rot, bmTan);
        bmTan = transpose(bmTan);
        t_normal = normalize(mul(bmTan, output.rotatedNorm));
    }

    float4 m_wPos = mul(world, pos);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = float3(1.0, 0.0, 0.0);
    float dt = dot(t_normal, right);
    float3 m_tangent, m_biTangent;
    if (dt == 0.0f) // -y方向正面を向いている面はnormal・upの結果がnanになるため処理を分ける
    {
        m_tangent = normalize(cross(t_normal, normalize(float3(0.1f, 0.8f, 0.1f)))); // ベクトルは適当
        m_biTangent = cross(t_normal, m_tangent);
    }
    else if (abs(abs(dt) - 1.0f) > 0.0f)
    {
        m_tangent = normalize(cross(t_normal, up));
        m_biTangent = cross(t_normal, m_tangent);
    }
    else
    {
        m_tangent = normalize(cross(t_normal, right));
        m_biTangent = cross(t_normal, m_tangent);
    }

    //float3 m_biNormal = cross(t_normal, m_tangent);
    float3 tEye = (float4(eyePos, 1) - m_wPos).xyz;
    float3 tLight = (float4(lightPos, 1) - m_wPos).xyz;
    output.vEyeDirection.x = abs(dot(m_tangent, tEye));
    output.vEyeDirection.y = dot(m_biTangent, tEye);
    output.vEyeDirection.z = abs(dot(t_normal, tEye));
    output.vEyeDirection = normalize(output.vEyeDirection);
    output.vLightDirection.x = dot(m_tangent, tLight);
    output.vLightDirection.y = dot(m_biTangent, tLight);
    output.vLightDirection.z = dot(t_normal, tLight);
    
    // 太陽位置が逆側になるとpixelshaderにおける太陽方向と法線マップの内積が負になり、全体が暗くなる現象の対策。
    // 太陽位置が初期状態でのsponza反対側の壁とライオンの描画結果が暗くなるが、この処理で明るい色に統一出来る。
    if (!output.isChara)
    {
        output.vLightDirection.x *= sign(output.vLightDirection.x);
        output.vLightDirection.x *= -1;
        output.vLightDirection.z *= sign(output.vLightDirection.z);
    }

    output.vLightDirection = normalize(output.vLightDirection);
    
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
    //norm.w = 0; // worldに平行移動成分が含まれている場合、法線が並行移動する。(この時モデルは暗くなる。なぜ？？)
    //output.norm = mul(world, m_normal);
    


    // タイリング対応しているuvについての処理は避ける。例えば負を正に変換する処理をすることで、テクスチャが斜めにゆがむ
    output.uv = uv;
    
    output.screenPosition = output.svpos;
    //output.worldPosition = pos; //mul(shadowPosMatrix, pos); // worldは単位行列なので乗算しない
    output.worldNormal = t_normal /*normalize(mul(world, m_normal).xyz)*/ /*normalize(mul(mul(mul(proj, view), world), norm).xyz)*/;
    
    //output.ray = normalize(pos.xyz - eyePos);

    //float3 rotEyepos = mul(rotation, eyePos);
    //float3 rotPos = mul(rotation, pos);
    //output.viewPos = mul(mul(view, world), rotEyepos).xyz - mul(mul(view, world), rotPos).xyz;
    //output.viewPos = mul(-rotation, mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz);
    
    float4 eye = float4(eyePos, 1);
    float4 vEye = mul(view, eye);
    
    output.wPos = m_wPos;
    
    float4 rotPos = mul(rotation, pos);
    float4 vPos = mul(mul(view, world), pos);
    output.viewPos = /*-eye */eye - output.wPos;
    
    output.oriWorldNorm = normalize(mul(world, oriNorm).xyz);
    //output.viewPos.z *= sign(output.viewPos.z);
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    //output.viewPos = mul(mul(view, world), eyePos).xyz - mul(mul(view, world), pos).xyz;
    
    return output;
}
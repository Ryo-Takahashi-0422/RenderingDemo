#include "FBXHeader.hlsli"

Output FBXVS
(float4 pos : POSITION,
    float4 norm : NORMAL_Vertex,
    float2 uv : TEXCOORD,
    uint3 boneno1 : BONE_NO_ZeroToTwo,
    uint3 boneno2 : BONE_NO_ThreeToFive,
    float3 boneweight1 : WEIGHT_ZeroToTwo,
    float3 boneweight2 : WEIGHT_ThreeToFive,
    uint index : SV_VertexID
)
{
    Output output; // ピクセルシェーダーに渡す値
    
    matrix bm1 = bones[boneno1[0]] * boneweight1[0];
    matrix bm2 = bones[boneno1[1]] * boneweight1[1];
    matrix bm3 = bones[boneno1[2]] * boneweight1[2];
    
    matrix bm = bm1 + bm2 + bm3;
    
    output.adjust = 200.0f;
    float3 lightPos;
    float adjustDirValue = 95.0f;
    lightPos.x = adjustDirValue * sunDIr.x;
    lightPos.y = -adjustDirValue * sunDIr.y;
    lightPos.z = adjustDirValue * sunDIr.z;
    output.light = lightPos;
    
    // 壁の法線値が-1になりssaoが計算出来くなる問題への対処
    float4 m_normal = norm;
 
    float3 t_normal;
    if (boneweight1[0] == 0 && boneweight2[2] == 0 && sponza)
    {
        m_normal.x *= sign(m_normal.x);
        m_normal.z *= sign(m_normal.z);

        output.worldPosition = pos;
        
        output.lvPos = mul(mul(oProj, shadowView), output.worldPosition);
        
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
        output.isChara = false;
        output.isEnhanceShadow = false;

        t_normal = normalize(mul(world, m_normal));
    }
    else
    {
        pos = mul(bm, pos);
        pos = mul(rotation, pos);
        output.worldPosition = mul(shadowPosMatrix, pos);        
        output.lvPos = mul(mul(mul(oProj, shadowView), shadowPosMatrix), pos);        
        output.trueDepth = length(output.worldPosition.xyz - lightPos) / output.adjust;
     
        float fixY = output.worldPosition.y + 5.0f;
        float enhance = output.worldPosition.y / fixY;
        lightPos /= enhance;

        output.isChara = true;
        output.isEnhanceShadow = true;

        bm = mul(rotation, bm);
        float4 rotatedNorm = mul(bm, m_normal);
        rotatedNorm = normalize(rotatedNorm);
        t_normal = normalize(rotatedNorm).xyz;
    }

    float4 m_wPos = mul(world, pos);
    output.viewSpacePos = mul(mul(view, world), pos);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = float3(1.0, 0.0, 0.0);
    float dt = dot(t_normal, right);
    float3 m_tangent, m_biTangent, s_Tangent, s_BiTangent; // s_...はスペキュラー用。m_...を使うと壁に三角形のアーティファクトが発生するため分岐した。効果としてはアーティファクトが目立たなくなっただけで、発生はしている。
    // TODO:アーティファクト解消する
    if (dt == 0.0f) // -y方向正面を向いている面はnormal・upの結果がnanになるため処理を分ける
    {
        m_tangent = normalize(cross(t_normal, normalize(float3(0.1f, 0.8f, 0.1f)))); // ベクトルは適当
        m_biTangent = cross(t_normal, m_tangent);
        
        s_Tangent = normalize(cross(t_normal, up));
        s_BiTangent = cross(t_normal, s_Tangent);
    }
    else if (abs(abs(dt) - 1.0f) > 0.0f)
    {
        m_tangent = normalize(cross(t_normal, up));
        m_biTangent = cross(t_normal, m_tangent);
        
        s_Tangent = normalize(cross(t_normal, up));
        s_BiTangent = cross(t_normal, s_Tangent);
    }
    else
    {
        m_tangent = normalize(cross(t_normal, right));
        m_biTangent = cross(t_normal, m_tangent);
        
        s_Tangent = normalize(cross(t_normal, right));
        s_BiTangent = cross(t_normal, s_Tangent);
    }

    float3 tEye = (float4(eyePos, 1) - m_wPos).xyz;   
    output.vEyeDirection.x = abs(dot(m_tangent, tEye));
    output.vEyeDirection.y = dot(m_biTangent, tEye);
    output.vEyeDirection.z = abs(dot(t_normal, tEye));
    output.vEyeDirection = normalize(output.vEyeDirection);
    
    float3 tLight = (float4(lightPos, 1) - m_wPos).xyz;
    
    // 法線マップ計算用
    output.vLightDirection.x = dot(m_tangent, tLight);
    output.vLightDirection.y = dot(m_biTangent, tLight);
    output.vLightDirection.z = dot(t_normal, tLight);
    
    // スペキュラー計算用
    lightPos.x *= -1;
    lightPos.z *= -1;
    tLight = (float4(lightPos, 1) - m_wPos).xyz;
    output.sLightDirection.x = dot(s_Tangent, tLight);
    output.sLightDirection.y = dot(s_BiTangent, tLight);
    output.sLightDirection.z = dot(t_normal, tLight);
    
    // 太陽位置が逆側になるとpixelshaderにおける太陽方向と法線マップの内積が負になり、全体が暗くなる現象の対策。
    // 太陽位置が初期状態でのsponza反対側の壁とライオンの描画結果が暗くなるが、この処理で明るい色に統一出来る。
    if (!output.isChara)
    {
        output.vLightDirection.x *= sign(output.vLightDirection.x);
        output.vLightDirection.x *= -1;
        output.vLightDirection.z *= sign(output.vLightDirection.z);
        output.vLightDirection = normalize(output.vLightDirection);
        
        output.sLightDirection.x *= sign(output.sLightDirection.x);
        output.sLightDirection.x *= -1;
        output.sLightDirection.z *= sign(output.sLightDirection.z);
        output.sLightDirection = normalize(output.sLightDirection);
    }
    else
    {
        output.vLightDirection = normalize(output.vLightDirection);
        output.sLightDirection = normalize(output.sLightDirection);
    }
    
    output.svpos = mul(mul(mul(proj, view), world), pos)/*mul(lightCamera, pos)*/;
  
    // タイリング対応しているuvについての処理は避ける。例えば負を正に変換する処理をすることで、テクスチャが斜めにゆがむ
    output.uv = uv;
    
    output.screenPosition = output.svpos;
    
    float4 eye = float4(eyePos, 1);
    float4 vEye = mul(view, eye);
    
    output.wPos = m_wPos;
    
    output.viewPos = eye - output.wPos;
    
    output.oriWorldNorm = normalize(mul(world, norm).xyz);
    
    output.directionalLight.color = float3(1, 1, 1);
    output.directionalLight.direction = float3(sunDIr.x, -sunDIr.y, sunDIr.z);
    
    output.pointLights[0].position = plPos1;
    output.pointLights[0].distance = 100.0f;

    output.pointLights[1].position = plPos2;
    output.pointLights[1].distance = 100.0f;
    
    output.weaken = -sunDIr.y;
    
    return output;
}
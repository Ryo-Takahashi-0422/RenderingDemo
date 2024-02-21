#include "FBXHeader.hlsli"
float SimpleGaussianBlur(Texture2D _texture, SamplerState _smp, float2 _uv /*, float dx, float dy*/)
{
    float ret = 0;
    
    float w, h, levels;
    _texture.GetDimensions(0, w, h, levels);
    float dx = 1.0f / w;
    float dy = 1.0f / h;

    // highest
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 2 * dy)) * 1;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 2 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 2 * dy)) * 1;
    // high
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 1 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 1 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 1 * dy)) * 4;
    // middle
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, 0 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, 0 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, 0 * dy)) * 36;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, 0 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, 0 * dy)) * 6;
    // low
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, -1 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, -1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, -1 * dy)) * 24;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, -1 * dy)) * 16;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, -1 * dy)) * 4;
    // lowest
    ret += _texture.Sample(smp, _uv + float2(-2 * dx, -2 * dy)) * 1;
    ret += _texture.Sample(smp, _uv + float2(-1 * dx, -2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(0 * dx, -2 * dy)) * 6;
    ret += _texture.Sample(smp, _uv + float2(1 * dx, -2 * dy)) * 4;
    ret += _texture.Sample(smp, _uv + float2(2 * dx, -2 * dy)) * 1;
 
    return ret / 256;
}
float4 FBXPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(0, 1, 0));
    float3 lightColor = float3(1, 1, 1);
    
    // �f�B�t���[�Y�v�Z
    float diffuseB = saturate(dot(light, input.norm.xyz));
    float tangentWeight = 1.0f;
    unsigned int biNormalWeight = 0; // 0��UV�V�[���������ڗ����Ȃ��Ȃ�
    float brightMin = 0.3f;
    float brightEmpha = 2.5f;
    
    // �^�C�����O�Ή�
    int uvX = abs(input.uv.x);
    int uvY = abs(input.uv.y);
    input.uv.x = abs(input.uv.x) - uvX;
    input.uv.y = abs(input.uv.y) - uvY;
        
    float3 normCol = normalmap.Sample(smp, input.uv);
    //return float4(normCol, 1);
    float3 normVec = normCol * 2.0f - 1.0f;
    normVec = normalize(normVec);
    //return float4(normVec, 1);
    float3 normal = input.tangent * tangentWeight * normVec.x + input.biNormal * normVec.y * biNormalWeight + input.normal * normVec.z;
    //return float4(normal, 1);
    
    //return float4(input.lightTangentDirection.xyz, 1);
    float bright = dot(input.lightTangentDirection.xyz, normal);
    bright = max(brightMin, bright);
    bright = saturate(bright * brightEmpha);
    
    float4 col = colormap.Sample(smp, input.uv);
    if (col.a == 0) discard; // �A���t�@�l��0�Ȃ瓧�߂�����
    
    //return col;
    
    float2 scrPos = input.screenPosition.xy / input.screenPosition.w; // �����Ώے��_�̃X�N���[����̍��W�B�������ԓ��̍��W�ɑ΂���w�ŏ��Z���āA�X�N���[���ɓ��e���邽�߂̗����̗̂̈�i-1��x��1�A-1��y��1������0��z��1�j�ɔ[�߂�B
    scrPos = 0.5 + float2(0.5, -0.5) * scrPos;
    float airZ = distance(input.worldPosition, eyePos) / 300; // areial�̉��s��300
    float4 air = airmap.Sample(smp, float3(scrPos, saturate(airZ)));
    float3 inScatter = air.xyz;

    float4 reslut = float4(bright * col.x, bright * col.y, bright * col.z, 1);
    //if(col.x !=0)
    //{
    
    
    float4 shadowPos = mul(mul(proj, shadowView), input.worldPosition);
    shadowPos.xyz /= shadowPos.w;
    float2 shadowUV = 0.5 + float2(0.5, -0.5) * shadowPos.xy /*(input.lvPos.xy / input.lvPos.w)*/;
    float shadowZ = /*shadowmap.Sample(smp, shadowUV)*/SimpleGaussianBlur(shadowmap, smp, shadowUV);
    float shadowFactor = 1;
    
    //float2 shadowValue = shadowmap.Sample(smp, shadowUV).xy;
    if (shadowPos.z - 0.00001f >= shadowZ) // ���ɃL�����N�^�[�̉e�ɉe�����Ă���B�e�̋��ڂ��ڂɂ��B
    {
        shadowFactor = min(0.3, (-sunDIr.y + 0.1));
    }
    //shadowPos.z /= 65;
    //float lz = input.lvPos.z;
    //if (lz > shadowValue.r && lz <= 1.0f)
    //{
    //    float depth_sq = shadowValue.x * shadowValue.x;
    //    float var = min(max(shadowValue.y - depth_sq, 0.0001f), 1.0f);
    //    float md = lz - shadowValue.x;
    //    float litFactor = var / (var + md * md);
    //    float3 shadowColor = reslut.xyz * 0.3f;
    //    reslut.xyz = lerp(shadowColor, reslut.xyz, litFactor);

    //}
    
    return reslut * shadowFactor + float4(inScatter, 0); /* + float4(diffuseB * diffuse.r, diffuseB * diffuse.g, diffuseB * diffuse.b, 1)*/;

}
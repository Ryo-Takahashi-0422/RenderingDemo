#include "SunHeader.hlsli"

vsOutput vs_main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    vsOutput output;
    //output.position = mul(float4(pos.x, pos.y, 0, 1), cameraPos);
    //output.position = mul(output.position, sunDir);
    float4 ori = float4(pos.x, pos.y, 0, 1);
    //float4 test = mul(mul(cameraPos, ori), sunDir);
    //
    
    ori = mul(billboard, ori);
    ori = mul(cameraPos, ori);
    
    ori = mul(sunDir, ori);

    //ori.z -= 0.1f;
    
    output.position = mul(mul(mul(proj, view), world), /*float4(pos.x, pos.y, 0, 1)*/ori);
    //output.position = mul(world, /*float4(pos.x, pos.y, 0, 1)*/ori);
    output.position.z = output.position.w; // •s—v??
    output.clipPos = output.position;
    output.clipPos = mul(scene, output.clipPos);
    //output.position = mul(cameraPos, output.position);
    //output.position = mul(sunDir, output.position);
    output.texCoord = uv;
    return output;
}
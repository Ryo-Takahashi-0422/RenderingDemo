#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    
    //float depth = length(input.worldPos - lightPos) / 100;
    //float depth2 = depth * depth;
    
    // [0],[1]:�L�����N�^�[�̂݌������߂Â����ꍇ�̒l�@�L�����N�^�[�̉e��n�ʂɔZ�����e���邽�߂�vsm
    // [2]:�S�Ēʏ�̌����ʒu�ł̋����@[0],[1]���L�����N�^�[�̃Z���t�V���h�E�`��ŗ��p�����depth��vsm�l���傫���Ȃ��ɉe�ɂȂ��Ă��܂��B
    return float4( /*0.0f*/input.depthAndLength.x/*depth*/, /*0.0f*/input.depthAndLength.y /*depth2*/, input.trueDepth, 1.0f);
}
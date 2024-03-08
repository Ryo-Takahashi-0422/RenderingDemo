#include "Shadow.hlsli"

float4 ps_main(vsOutput input) : SV_TARGET
{
    float3 oLightPos = lightPos;
    // 2024/2/28���_�Asponza�̑��Ԃ�ԕr�ȂǍ��x��߂̃I�u�W�F�N�g�̉e�������Ȃ���ɑ΂����i�͈ȉ��ȊO�ɂȂ�...�ȉ��̂悤�ɂ�����I�u�W�F�N�g�Ƀ��C�g���߂Â��ĉe����������ƁA���z���x��߂̂Ƃ��ɊO�ǂ������ђʂ��Ă��܂�...
    // pix���E�������_�C���f�b�N�X�ԍ��ŉe����������I�u�W�F�N�g�𒼂Ɏw�肵�Ă���B���Ȃ݂ɃL�����N�^�[�`�掞�ɂ̓C���f�b�N�X��0�J�n����25714�ȉ��Ɏ��܂邽�߁A�L�����N�^�[�̉e�������o���Ă���B����͂�������ȊO�Ɏv�����Ȃ�...
    // 0...25714 
    // 49897...77462 
    // 77496...91841 �ǃ|�[��
    // 229613...233693 
    if (input.index <= 25714 || (49897 <= input.index && input.index <= 77462) || (77496 <= input.index && input.index <= 91841) || (229613 <= input.index && input.index <= 233693))
    {
        float newY = input.worldPos.y + 5.0f;
        float div = oLightPos.y / newY;
        oLightPos /= div;
    }
    
    float depth = length(input.worldPos - oLightPos) / input.adjust;
    float depth2 = depth * depth;
    
    // [0],[1]:�L�����N�^�[�̂݌������߂Â����ꍇ�̒l�@�L�����N�^�[�̉e��n�ʂɔZ�����e���邽�߂�vsm
    // [2]:�S�Ēʏ�̌����ʒu�ł̋����@[0],[1]���L�����N�^�[�̃Z���t�V���h�E�`��ŗ��p�����depth��vsm�l���傫���Ȃ��ɉe�ɂȂ��Ă��܂��B
    return float4( /*0.0f*/ /*input.depthAndLength.x*/depth, /*0.0f*/ /*input.depthAndLength.y*/depth2, input.trueDepth, input.specialObj);
}
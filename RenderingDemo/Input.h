#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
// �L�[�ő吔
#define KEY_MAX 256

class Input
{
private:
	// �C���v�b�g�̐���
	HRESULT CreateInput(void);
	// �L�[�f�o�C�X�̐���
	HRESULT CreateKey(void);
	// �L�[�t�H�[�}�b�g�̃Z�b�g
	HRESULT SetKeyFormat(void);
	// �L�[�̋������x���̃Z�b�g
	HRESULT SetKeyCooperative(void);

	// �E�B���h�E
	PrepareRenderingWindow* pWin;
	// �Q�ƌ���
	HRESULT result;
	// �C���v�b�g
	LPDIRECTINPUT8 input;
	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 key;
	// �L�[���
	BYTE keys[KEY_MAX];
	// �O�̃L�[���
	BYTE olds[KEY_MAX];

public:
	// �R���X�g���N�^
	Input(PrepareRenderingWindow* win);
	// �f�X�g���N�^
	~Input();
	// �L�[����
	bool CheckKey(UINT index);
	// �g���K�[�̓���
	bool TriggerKey(UINT index);

};
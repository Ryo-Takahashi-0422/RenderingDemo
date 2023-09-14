#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
// キー最大数
#define KEY_MAX 256

class Input
{
private:
	// インプットの生成
	HRESULT CreateInput(void);
	// キーデバイスの生成
	HRESULT CreateKey(void);
	// キーフォーマットのセット
	HRESULT SetKeyFormat(void);
	// キーの協調レベルのセット
	HRESULT SetKeyCooperative(void);

	// ウィンドウ
	PrepareRenderingWindow* pWin;
	// 参照結果
	HRESULT result;
	// インプット
	LPDIRECTINPUT8 input;
	// インプットデバイス
	LPDIRECTINPUTDEVICE8 key;
	// キー情報
	BYTE keys[KEY_MAX];
	// 前のキー情報
	BYTE olds[KEY_MAX];

public:
	// コンストラクタ
	Input(PrepareRenderingWindow* win);
	// デストラクタ
	~Input();
	// キー入力
	bool CheckKey(UINT index);
	// トリガーの入力
	bool TriggerKey(UINT index);

};
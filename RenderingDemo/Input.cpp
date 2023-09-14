#include <stdafx.h>
#include "Input.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// 解放マクロ
#define Release(X) { if((X) != nullptr) (X)->Release(); (X) = nullptr; }

// コンストラクタ
Input::Input(PrepareRenderingWindow* win) :
    pWin(win), result(S_OK), input(nullptr), key(nullptr)
{
    memset(&keys, 0, sizeof(keys));
    memset(&olds, 0, sizeof(olds));

    CreateInput();
    CreateKey();
    SetKeyFormat();
    SetKeyCooperative();
}
// デストラクタ
Input::~Input()
{
    Release(key);
    Release(input);
    delete pWin;
}

// インプットの生成
HRESULT Input::CreateInput(void)
{
    result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&input), NULL);

    return result;
}

// キーデバイスの生成
HRESULT Input::CreateKey(void)
{
    result = input->CreateDevice(GUID_SysKeyboard, &key, NULL);

    return result;
}

// キーフォーマットのセット
HRESULT Input::SetKeyFormat(void)
{
    result = key->SetDataFormat(&c_dfDIKeyboard);

    return result;
}

// キーの協調レベルのセット
HRESULT Input::SetKeyCooperative(void)
{
    result = key->SetCooperativeLevel(pWin->GetHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

    //入力デバイスへのアクセス権利を取得
    key->Acquire();

    return result;
}

// キー入力
bool Input::CheckKey(UINT index)
{
    //チェックフラグ
    bool flag = false;

    //キー情報を取得
    key->GetDeviceState(sizeof(keys), &keys);
    if (keys[index] & 0x80)
    {
        flag = true;
    }
    olds[index] = keys[index];
    
    return flag;
}

// トリガーの入力
bool Input::TriggerKey(UINT index)
{
    //チェックフラグ
    bool flag = false;

    //キー情報を取得
    key->GetDeviceState(sizeof(keys), &keys);
    if ((keys[index] & 0x80) && !(olds[index] & 0x80))
    {
        flag = true;
    }
    olds[index] = keys[index];

    return flag;
}
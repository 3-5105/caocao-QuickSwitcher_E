#pragma once
#include <string>
#include <Windows.h>

#define MAX_PATH_LENGTH 260
#define MAX_TITLE_LENGTH 256
#define MAX_CLASS_LENGTH 256
#define MAX_NAME_LENGTH 256


struct UserWinInfo {
    HWND hWnd;
    HICON hIcon;
    wchar_t winTitle[MAX_TITLE_LENGTH];
    wchar_t exePath[MAX_PATH_LENGTH];
};

struct UserItemInfo {
    wchar_t name[MAX_NAME_LENGTH];     // 缁勫悕锛堣鍒欏悕绉版垨绐楀彛鏍囬锛?
    UserWinInfo* windows;              // 缁勫唴鐨勭獥鍙ｆ暟缁?
    int windowCount;                   // 绐楀彛鏁伴噺
    unsigned int hotkey = 5105;                    // 缁勭殑蹇嵎閿?
};


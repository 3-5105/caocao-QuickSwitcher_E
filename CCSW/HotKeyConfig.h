#pragma once
#include "SimpleIni/SimpleIni.h"
#include <string>
#include <unordered_map>
#include <sstream>
#include <windows.h>
#include <vector>

#define I_Alt 0
#define I_Ctrl 1
#define I_Shift 2
#define I_Win 3

#define I_ListType_Tree 0
#define I_ListType_Menu 1

#define WM_SHOWLIST WM_USER + 5

// 在类定义之前添加结构体
struct HotKeyCombination {
    bool modifiers[4];  // 存储修饰键（Alt, Ctrl, Shift, Win）
    char keys[4];      // 存储主键（A-Z, F1-F12等）
    bool followMouse = false; // 是否跟随鼠标
    bool UP_Active = false; // 是否激活UP
    int ListType = I_ListType_Tree; // 列表类型
    
    // 构造函数，初始化数组
    HotKeyCombination() {
        for (int i = 0; i < 4; ++i) {
            modifiers[i] = 0;
            keys[i] = 0;
        }
    }
};

// 定义热键类型枚举
enum HKType {
    HK_GLOBAL_HOOK,   // 全局钩子
    HK_SYSTEM_HOTKEY, // 系统热键
    HK_POLLING        // 轮询
};

// 新增函数声明
int GetKey(const std::wstring& keys, HotKeyCombination *combination);
void LoadHotKeyCombinations();
void CallList(const HotKeyCombination& combo);
void SetPolling();
void SetupHotKey();
bool SetupSystemHotKey();
void CleanupSystemHotKey();
bool RegisterHotKeyCombination(const HotKeyCombination& combo, int i);

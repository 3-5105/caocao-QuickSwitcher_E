#pragma once
#include <vector>
#include <string>
#include <Windows.h>

#define MAX_PATH_LENGTH 260
#define MAX_TITLE_LENGTH 256
#define MAX_CLASS_LENGTH 256

// 窗口信息结构体
struct WinInfo {
    HWND hWnd = nullptr;               // 窗口句柄
    HICON hIcon = nullptr;             // 窗口图标
    std::wstring winTitle;             // 窗口标题
    std::wstring className;            // 窗口类名
    std::wstring exePath;              // 可执行文件路径
    DWORD style = 0;                   // 窗口样式
    bool isTopMost = false;            // 是否置顶
    bool isNew = false;                // 是否新窗口
    wchar_t hotkey = 0;                // 快捷键
    std::wstring ruleName;             // 规则名称

    // 默认构造函数
    WinInfo() = default;

    // 移动构造函数
    WinInfo(WinInfo&& other) noexcept = default;

    // 移动赋值运算符
    WinInfo& operator=(WinInfo&& other) noexcept = default;

    // 拷贝构造函数
    WinInfo(const WinInfo& other) = default;

    // 拷贝赋值运算符
    WinInfo& operator=(const WinInfo& other) = default;
};

// 项目信息结构体
struct ItemInfo {
    std::wstring name;                 // 组名（规则名称或窗口标题）
    std::vector<WinInfo> windows;      // 组内的窗口列表
    bool isTopMost = false;            // 组的置顶状态
    bool isNew = false;                // 组的新建状态
    wchar_t hotkey = 0;                // 组的快捷键
    std::wstring ruleName;             // 规则名称

    // 默认构造函数
    ItemInfo() = default;

    // 移动构造函数
    ItemInfo(ItemInfo&& other) noexcept = default;

    // 移动赋值运算符
    ItemInfo& operator=(ItemInfo&& other) noexcept = default;

    // 拷贝构造函数
    ItemInfo(const ItemInfo& other) = default;

    // 拷贝赋值运算符
    ItemInfo& operator=(const ItemInfo& other) = default;

    // 析构函数
    ~ItemInfo() = default;
};

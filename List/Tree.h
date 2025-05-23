#pragma once
#include <Windows.h>
#include <UserInfo.h>

#pragma comment(lib, "uxtheme.lib")

// 主题模式枚举
enum class ThemeMode {
    Light,
    Dark
};

// 全局变量声明
extern HWND g_hTreeWindow;
extern UserItemInfo* g_treeItems;
extern int g_treeItemCount;
extern bool showingTree;

// 仅在此文件中使用的静态变量
static HWND g_hTreeView = NULL;
static WNDPROC g_pfnOrigTreeViewProc = NULL;
static const wchar_t* TREE_WINDOW_CLASS = L"ListTreeWindow";
static ThemeMode g_currentTheme = ThemeMode::Light;

// 清理资源的函数
void WINAPI CleanupTree();

// 创建树状图窗口
BOOL TreeInit(HINSTANCE hInstance);

// 填充树状视图
void PopulateTreeView(UserItemInfo* treeItems, int treeItemCount);

// 设置树状图窗口位置
void SetTreeWindowPos(POINT pt, bool followMouse);

// 设置主题模式
void SetTreeTheme(ThemeMode mode);


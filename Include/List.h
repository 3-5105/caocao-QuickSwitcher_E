#pragma once
#include <Windows.h>
#include <InfoManager.h>
#include <UserInfo.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <List.h>

#define WM_MENU_WHEEL_UP   (WM_USER + 1)
#define WM_MENU_WHEEL_DOWN (WM_USER + 2)

extern "C" __declspec(dllimport) HWND WINAPI List_Menu(
	POINT pt, bool followMouse = false,
	UserItemInfo* items = nullptr, int itemCount = 0);

extern "C" __declspec(dllexport) HWND WINAPI List_Tree(
	POINT pt, bool followMouse = false,
	UserItemInfo* treeItems = nullptr, int treeItemCount = 0);

extern "C" __declspec(dllexport) void WINAPI LoadMaxTitleLength(int maxTitleLength);

extern "C" __declspec(dllexport) void WINAPI List_SetTheme(bool darkMode);

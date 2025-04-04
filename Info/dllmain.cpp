// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "Info.h"
#include "Item.h"
#include "Manager.h"
#include "Rule.h"
#include <algorithm>
#include <map>
#include <string>
#include <UserInfo.h>



// 全局规则管理器
static WindowRuleManager g_ruleManager;
static WindowManager* g_windowManager = nullptr;
static WindowItem* g_windowItem = nullptr;
static std::vector<WinInfo> windows;
static std::map<std::wstring, ItemInfo> items;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) { 
        case DLL_PROCESS_ATTACH:
        {
            // 加载默认配置
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(nullptr, path, MAX_PATH);
            std::wstring exePath = path;
            std::wstring iniPath = exePath.substr(0, exePath.find_last_of(L'\\') + 1) + L"setting.ini";
            g_ruleManager.LoadRules(iniPath.c_str());
            g_windowManager = new WindowManager();
            g_windowItem = new WindowItem();
            break;
        }
        case DLL_PROCESS_DETACH:
            delete g_windowManager;
            g_windowManager = nullptr;
            delete g_windowItem;
            g_windowItem = nullptr;
            break;
    }
    return TRUE;
}

// 释放内存
extern "C" __declspec(dllexport) void WINAPI FreeWindowList(UserItemInfo* list, int count) {
    if (list) {
        for(int i = 0; i < count; i++)
            delete[] list[i].windows;
    }
    // 移除isNew为false的项
    windows.erase(std::remove_if(windows.begin(), windows.end(),
        [](const WinInfo& info) { return !info.isNew; }),
        windows.end());
}

// 列出窗口
extern "C" __declspec(dllexport) bool WINAPI EnumerateWindows(UserItemInfo** outItems, int* outItemCount) {
    if (!g_windowManager->GetWindowList(windows)) 
        return false;

    if (!g_ruleManager.ApplyRules(windows))
        return false;

    // 清空items
    items.clear();

    // 手动分组逻辑
    for (const auto& win : windows) {
        if(win.winTitle == L"")
            continue;
        if(!win.isNew)
            continue;

        bool grouped = false;
        
        // 尝试按规则名称分组
        if (!win.ruleName.empty()) {
            for (auto& pair : items) {
                if (pair.second.name == win.ruleName) {
                    pair.second.windows.push_back(win);
                    grouped = true;
                    pair.second.hotkey = win.hotkey;
                    break;
                }
            }
        }
        
        // 如果未按规则分组，尝试按窗口标题分组
        if (!grouped) {
            for (auto& pair : items) {
                if (pair.second.windows.size() > 0 && 
                    pair.second.windows[0].winTitle == win.winTitle) {
                    pair.second.windows.push_back(win);
                    grouped = true;
                    pair.second.hotkey = win.hotkey;
                    break;
                }
            }
        }
        
        // 如果仍未分组，则创建新组
        if (!grouped) {
            // 直接在map中构造新项
            auto& newItem = items[win.ruleName.empty() ? win.winTitle : win.ruleName];
            newItem.name = win.ruleName.empty() ? win.winTitle : win.ruleName;
            newItem.windows.push_back(win);
            newItem.hotkey = win.hotkey;
        }
    }

    // 计算需要的内存大小
    int totalItems = static_cast<int>(items.size());
    *outItemCount = totalItems;
    
    // 分配内存
    *outItems = new UserItemInfo[totalItems];
    if (!*outItems) return false;
    
    int currentIndex = 0;
    
    // 首先添加置顶的项目
    for (const auto& pair : items) {
        const ItemInfo& item = pair.second;
        if (item.isTopMost) {
            UserItemInfo& userItem = (*outItems)[currentIndex];
            
            // 复制名称
            wcscpy_s(userItem.name, MAX_NAME_LENGTH, item.name.c_str());
            userItem.hotkey = item.hotkey;
            
            // 分配并复制窗口信息
            userItem.windowCount = static_cast<int>(item.windows.size());
            userItem.windows = new UserWinInfo[userItem.windowCount];
            
            for (int i = 0; i < userItem.windowCount; i++) {
                const auto& win = item.windows[i];
                UserWinInfo& userWin = userItem.windows[i];
                
                userWin.hWnd = win.hWnd;
                userWin.hIcon = win.hIcon;
                wcscpy_s(userWin.winTitle, MAX_TITLE_LENGTH, win.winTitle.c_str());
                wcscpy_s(userWin.exePath, MAX_PATH_LENGTH, win.exePath.c_str());
            }
            
            currentIndex++;
        }
    }

    // 创建临时数组用于排序非置顶项目
    struct TempItem {
        wchar_t hotkey;
        const ItemInfo* itemInfo;
    };
    std::vector<TempItem> nonTopMostItems;
    
    for (const auto& pair : items) {
        const ItemInfo& item = pair.second;
        if (!item.isTopMost) {
            nonTopMostItems.push_back({item.hotkey, &item});
        }
    }

    // 按快捷键排序
    std::sort(nonTopMostItems.begin(), nonTopMostItems.end(),
        [](const auto& a, const auto& b) {
            if (a.hotkey == 0) return false;
            if (b.hotkey == 0) return true;
            return a.hotkey < b.hotkey;
        });

    // 添加排序后的非置顶项目
    for (const auto& tempItem : nonTopMostItems) {
        const ItemInfo& item = *tempItem.itemInfo;
        UserItemInfo& userItem = (*outItems)[currentIndex];
        
        wcscpy_s(userItem.name, MAX_NAME_LENGTH, item.name.c_str());
        userItem.hotkey = item.hotkey;
        
        userItem.windowCount = static_cast<int>(item.windows.size());
        userItem.windows = new UserWinInfo[userItem.windowCount];
        
        for (int i = 0; i < userItem.windowCount; i++) {
            const WinInfo& win = item.windows[i];
            UserWinInfo& userWin = userItem.windows[i];
            
            userWin.hWnd = win.hWnd;
            userWin.hIcon = win.hIcon;
            wcscpy_s(userWin.winTitle, MAX_TITLE_LENGTH, win.winTitle.c_str());
            wcscpy_s(userWin.exePath, MAX_PATH_LENGTH, win.exePath.c_str());
        }
        
        currentIndex++;
    }

    return true;
}


// 重载规则
extern "C" __declspec(dllexport) bool WINAPI ReloadRules(const wchar_t* iniPath) {
    try {
        return g_ruleManager.LoadRules(iniPath);
    }
    catch (...) {
        return false;
    }
} 


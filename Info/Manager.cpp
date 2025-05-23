#include "Manager.h"
#include <string>
#include <Windows.h>
#include <algorithm>
#include <shellapi.h>
 
using namespace std;

WindowManager::WindowManager() {
}

WindowManager::~WindowManager() {
}

bool WindowManager::GetWindowList(std::vector<WinInfo>& outWindows) {
    // 重置所有窗口的isNew标志
    for (auto& pair : outWindows)
        pair.isNew = false;

    // 枚举窗口
    struct EnumWindowsData {
        WindowManager* windowManager;
        std::vector<WinInfo>* outWindows;
    };
    EnumWindowsData data = {
        this,
        &outWindows
    };
    WNDENUMPROC enumProc = [](HWND hwnd, LPARAM lParam)->BOOL CALLBACK {
        auto data = (EnumWindowsData*)lParam;
        data->windowManager->ProcessWindow(hwnd, *data->outWindows);
        return TRUE;
    };
    if (!EnumWindows(enumProc, reinterpret_cast<LPARAM>(&data)))
        return false;



    return true;
} 

bool WindowManager::ProcessWindow(HWND hwnd, std::vector<WinInfo>& windows) {
    // 在所有组中查找窗口
    for (auto& window : windows) {
        if (window.hWnd == hwnd) {
            // 获取标题
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            if (wcslen(title) == 0) {
                return false;
            }
            window.winTitle = title;
            window.isNew = true;
            return true;
        }
    }

    // 检查窗口可见性
    if (!IsWindowVisible(hwnd)) {
        return true;
    }

    // 新窗口，获取基本信息
    WinInfo info;
    if (!InitializeWindowInfo(hwnd, info)) {
        return true;
    }

    info.isNew = true;
    windows.push_back(std::move(info));
    return true;
}

bool WindowManager::InitializeWindowInfo(HWND hwnd, WinInfo& info) {
    // 检查基本条件
    info.isNew = true;

    // 获取窗口标题
    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    if (wcslen(title) == 0) {
        return false;
    }

    // 初始化基本信息
    info = WinInfo();  // 清零所有字段
    info.hWnd = hwnd;
    
    // 获取进程路径（在获取图标之前）
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    if (HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId)) {
        wchar_t exePath[MAX_PATH];
        DWORD pathSize = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, exePath, &pathSize)) {
            info.exePath = exePath;
        }
        CloseHandle(hProcess);
    }

    info.hIcon = GetWindowIcon(info);  // 现在传入info对象
    info.winTitle = title;

    // 获取类名
    wchar_t className[256];
    GetClassNameW(hwnd, className, 256);
    info.className = className;

    // 获取窗口样式
    info.style = GetWindowLongPtr(hwnd, GWL_STYLE);

    return true;
}

HICON WindowManager::GetWindowIcon(WinInfo& info) {
    // 灏濊瘯浠庣獥鍙ｈ幏鍙栧浘鏍?
    HICON hIcon = (HICON)SendMessage(info.hWnd, WM_GETICON, ICON_BIG, 0);
    if (!hIcon) {
        hIcon = (HICON)GetClassLongPtr(info.hWnd, GCLP_HICON);
    }

    // 濡傛灉浠庣獥鍙ｈ幏鍙栧け璐ワ紝灏濊瘯浠巈xe鏂囦欢鑾峰彇鍥炬爣
    if (!hIcon && !info.exePath.empty()) {
        ExtractIconExW(info.exePath.c_str(), 0, &hIcon, nullptr, 1);
    }

    // 濡傛灉浠嶇劧澶辫触锛屼娇鐢ㄧ郴缁熼粯璁ゅ浘鏍?
    if (!hIcon) {
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    return hIcon;
}


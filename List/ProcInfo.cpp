#include "common.h"
#include <CommCtrl.h>

#pragma comment(lib, "user32.lib")

// 创建菜单图标位图
HBITMAP CreateMenuIconBitmap(HICON hIcon, COLORREF bgColor) {
    if (!hIcon) return NULL; // 图标无效

    // 获取屏幕 DPI
    HDC hdc = GetDC(NULL);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    // 根据 DPI 计算图标大小
    int iconSize = MulDiv(32, dpiX, 96);

    // 创建位图
    HBITMAP hBitmap = NULL;

    // 创建32位带alpha通道的位图
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = iconSize;
    bmi.bmiHeader.biHeight = -iconSize;  // 负值表示从上到下的位图
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // 创建兼容 DC
    hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // 设置背景颜色
    if (bgColor != CLR_NONE) {
        HBRUSH hBrush = CreateSolidBrush(bgColor);
        RECT rc = { 0, 0, iconSize, iconSize };
        FillRect(hdcMem, &rc, hBrush);
        DeleteObject(hBrush);
    }

    // 绘制图标
    DrawIconEx(hdcMem, 0, 0, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);

    return hBitmap; // 返回位图
}

// 获取 DPI 的函数
int CGetDpiForWindow(HWND hwnd) {
    // 使用 CGetDpiForWindow 函数，适用于 Windows 10 1607 及以上版本
    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    typedef UINT (WINAPI *CGetDpiForWindow_t)(HWND);
    CGetDpiForWindow_t CGetDpiForWindow = (CGetDpiForWindow_t)GetProcAddress(hUser32, "CGetDpiForWindow");
    
    if (CGetDpiForWindow) {
        return (int)CGetDpiForWindow(hwnd); // 返回 DPI
    }
    
    // 兼容旧版
    HDC hdc = GetDC(NULL);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return dpi; // 返回 DPI
}

// 根据 DPI 缩放值
int ScaleForDpi(int value, int dpi) {
    return MulDiv(value, dpi, 96); // 根据 DPI 计算缩放值
}

// 计算显示器中心点
POINT GetMonitorCenterPoint(POINT pt) {
    POINT centerPt = pt;
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    if (GetMonitorInfo(hMonitor, &mi)) {
        centerPt.x = (mi.rcMonitor.left + mi.rcMonitor.right) / 2; // 计算中心点 X 坐标
        centerPt.y = (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2; // 计算中心点 Y 坐标
    }
    return centerPt; // 返回中心点
}

// 最大标题长度
static int g_maxTitleLength = 20;

// 设置最大标题长度
wchar_t* ProcTitle(wchar_t* Title) {
    if (!Title) 
        return Title;
    
    // 处理 %UXXXX 格式的 Unicode 字符
    wchar_t* tempTitle = _wcsdup(Title);
    if (tempTitle) {
        int len = (int)wcslen(tempTitle);
        int j = 0;
        
        for (int i = 0; i < len; i++) {
            if (tempTitle[i] == L'%' && i + 5 < len && tempTitle[i+1] == L'U') {
                // 尝试解析 %UXXXX 格式的 Unicode 字符
                wchar_t hexStr[5] = { tempTitle[i+2], tempTitle[i+3], tempTitle[i+4], tempTitle[i+5], L'\0' };
                wchar_t* endPtr;
                unsigned long unicodeValue = wcstoul(hexStr, &endPtr, 16);
                
                // 如果成功解析了 Unicode 值
                if (*endPtr == L'\0') {
                    Title[j++] = (wchar_t)unicodeValue;
                    i += 5; // 跳过已处理的字符
                } else {
                    Title[j++] = tempTitle[i];
                }
            } else {
                Title[j++] = tempTitle[i];
            }
        }
        
        Title[j] = L'\0'; // 确保字符串正确终止
        free(tempTitle);
    }
    
    if (wcslen(Title) > g_maxTitleLength) {
        Title[g_maxTitleLength - 3] = L'.';
        Title[g_maxTitleLength - 2] = L'.';
        Title[g_maxTitleLength - 1] = L'.';
        Title[g_maxTitleLength] = L'\0'; // 截断标题
    }
    
    return Title; // 返回标题
}

// 获取最大标题长度
extern "C" __declspec(dllexport) void WINAPI LoadMaxTitleLength(int maxTitleLength) {
    g_maxTitleLength = maxTitleLength; // 设置最大标题长度
}

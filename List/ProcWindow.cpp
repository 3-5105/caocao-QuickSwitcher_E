#include "common.h"

// 检查窗口是否挂起
bool IsWindowHung(HWND hWnd) {
    DWORD_PTR dwResult;
    LRESULT ok = SendMessageTimeout(hWnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 1000, &dwResult);
    return (ok == 0); // 返回是否挂起
}

// 尝试设置前景窗口
HWND AttemptSetForeground(HWND hWnd, HWND origForeground) {
    SetForegroundWindow(hWnd);
    if (GetForegroundWindow() == hWnd)
        return hWnd; // 成功
    return NULL; // 失败
}

// 发送键盘事件
void KeyEvent(BYTE vk, bool press) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT)); // 发送输入
}



// 设置前景窗口的扩展函数
HWND SetForegroundWindowEx(HWND aTargetWindow, DWORD TID) {
    if (!aTargetWindow) return NULL;
    DWORD target_thread = GetWindowThreadProcessId(aTargetWindow, NULL);
    if (target_thread != TID && IsWindowHung(aTargetWindow))
        return NULL;
    HWND orig_foreground_wnd = GetForegroundWindow();
    if (IsIconic(aTargetWindow))
        ShowWindow(aTargetWindow, SW_RESTORE);
    if (aTargetWindow == orig_foreground_wnd)
        return aTargetWindow;
    HWND new_foreground_wnd = NULL;
    // 强制激活
    new_foreground_wnd = AttemptSetForeground(aTargetWindow, orig_foreground_wnd);
    if (new_foreground_wnd)
        return new_foreground_wnd;
    bool is_attached_my_to_fore = false;
    bool is_attached_fore_to_target = false;
    DWORD fore_thread = 0;
    if (orig_foreground_wnd) {
        fore_thread = GetWindowThreadProcessId(orig_foreground_wnd, NULL);
        if (fore_thread && TID != fore_thread && !IsWindowHung(orig_foreground_wnd))
            is_attached_my_to_fore = AttachThreadInput(TID, fore_thread, TRUE);
        if (fore_thread && target_thread && fore_thread != target_thread)
            is_attached_fore_to_target = AttachThreadInput(fore_thread, target_thread, TRUE);
    }
    // 重试
    for (int i = 0; i < 5; ++i) {
        new_foreground_wnd = AttemptSetForeground(aTargetWindow, orig_foreground_wnd);
        if (new_foreground_wnd)
            break;
        Sleep(30); // 等待
    }
    // ALT 激活
    if (!new_foreground_wnd) {
        KeyEvent(VK_MENU, true);  // 按下 ALT
        KeyEvent(VK_MENU, false); // 释放 ALT
        KeyEvent(VK_MENU, true);
        KeyEvent(VK_MENU, false);
        new_foreground_wnd = AttemptSetForeground(aTargetWindow, orig_foreground_wnd);
    }
    // 恢复线程状态
    if (!is_attached_my_to_fore)
        AttachThreadInput(TID, fore_thread, FALSE);
    if (!is_attached_fore_to_target)
        AttachThreadInput(fore_thread, target_thread, FALSE);
    if (new_foreground_wnd) {
        BringWindowToTop(aTargetWindow);
        SendMessage(aTargetWindow, WM_ACTIVATE, WA_ACTIVE, 0);
        return new_foreground_wnd;
    }
    return NULL; // 激活失败
}


// 激活窗口 * https://learn.microsoft.com/zh-cn/windows/win32/api/winuser/nf-winuser-setforegroundwindow
void ActivateWindow(HWND hwndTarget) {
    if (IsWindow(hwndTarget)) {
        // 保存当前激活的窗口
        HWND hwndPrev = GetForegroundWindow();

        if (IsIconic(hwndTarget))
            ShowWindow(hwndTarget, SW_RESTORE); // 还原窗口

        // 激活目标窗口
        SetForegroundWindowEx(hwndTarget, GetCurrentThreadId());

        // 给原窗口发送 WM_ACTIVATE 消息
        if (hwndPrev && hwndPrev != hwndTarget) {
            SendMessage(hwndPrev, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hwndTarget);
        }
    }
}
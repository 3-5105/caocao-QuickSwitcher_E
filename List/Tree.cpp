#include "TreeViewHelper.h"

// 自定义树状图窗口过程
static LRESULT CALLBACK TreeViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEWHEEL:
    {
        // 获取滚轮方向
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (delta == 0) break;

        // 获取当前选中项
        HTREEITEM hCurrent = TreeView_GetSelection(hwnd);
        if (!hCurrent) break;

        if (delta > 0) {
            // 向上滚动
            // 检查是否是父项的最后一个子项
            HTREEITEM hParent = TreeView_GetParent(hwnd, hCurrent);
            if (hParent) {
                HTREEITEM hPrev = TreeView_GetPrevSibling(hwnd, hCurrent);
                if (!hPrev) { // 如果是第一个子项，先执行上键逻辑，然后关闭父项
                    TreeView_Expand(hwnd, hParent, TVE_COLLAPSE);
                    return 0;
                }
            }
            else {
                HTREEITEM hPrev = TreeView_GetPrevVisible(hwnd, hCurrent);
                if (hPrev) {  // 检查上一个项是否有子项
                    HTREEITEM hPrevChild = TreeView_GetChild(hwnd, hPrev);
                    if (hPrevChild)  // 如果有子项，先展开此项
                        TreeView_Expand(hwnd, hPrev, TVE_EXPAND);
                }
            }
            // 正常向上滚动
            PostMessage(hwnd, WM_KEYDOWN, VK_UP, 0);
        }
        else {
            // 向下滚动
            // 检查当前项是否有子项
            HTREEITEM hChild = TreeView_GetChild(hwnd, hCurrent);
            if (hChild) {
                // 如果有子项，先展开此项
                TreeView_Expand(hwnd, hCurrent, TVE_EXPAND);
            }
            else {
                // 检查是否是父项的最后一个子项
                HTREEITEM hParent = TreeView_GetParent(hwnd, hCurrent);
                if (hParent) {
                    HTREEITEM hNext = TreeView_GetNextSibling(hwnd, hCurrent);
                    if (!hNext) {
                        // 如果是最后一个子项，先执行下键逻辑，再收起父项
                        TreeView_Expand(hwnd, hParent, TVE_COLLAPSE);
                        PostMessage(hwnd, WM_KEYDOWN, VK_DOWN, 0);
                        return 0;
                    }
                }
            }
            // 正常向下滚动
            PostMessage(hwnd, WM_KEYDOWN, VK_DOWN, 0);
        }
        return 0; // 已处理，阻止默认行为
    }
    }

    // 调用原始窗口过程处理其他消息
    return CallWindowProc(g_pfnOrigTreeViewProc, hwnd, msg, wParam, lParam);
}

// 窗口过程
LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // 初始化通用控件
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_TREEVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        // 获取当前 DPI
        int dpi = CGetDpiForWindow(hwnd);



        // 创建树状视图控件
        g_hTreeView = CreateWindowEx(
            0, WC_TREEVIEW, L"",
            WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_NONEVENHEIGHT,
            0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(0), 0);
/*
                if (isdark)
    {
        auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~WS_BORDER;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        auto styleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        styleEx &= ~WS_EX_CLIENTEDGE;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, styleEx);
    }
    else
    {
        auto style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style |= WS_BORDER;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        auto styleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        styleEx |= WS_EX_CLIENTEDGE;
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, styleEx);
    }
*/
        // 根据 DPI 缩放设置行高
        SetTreeRowHeight(40); 

        // 设置初始主题
        SetTreeTheme(g_currentTheme);

        // 设置树状图控件的子类化
        if (g_hTreeView) 
            g_pfnOrigTreeViewProc = (WNDPROC)SetWindowLongPtr(
                g_hTreeView, GWLP_WNDPROC, (LONG_PTR)TreeViewSubclassProc);
        return 0;
    }

    case WM_SIZE:
        // 调整树状视图控件大小
        if (g_hTreeView) {
            int margin = ScaleForDpi(5, CGetDpiForWindow(hwnd));
            SetWindowPos(g_hTreeView, NULL,
                margin, margin,
                LOWORD(lParam) - 2 * margin,
                HIWORD(lParam) - 2 * margin,
                SWP_NOZORDER);
        }
        return 0;

    case WM_NOTIFY:
        // 处理树状视图通知消息
        if (((LPNMHDR)lParam)->idFrom == 1)
            switch (((LPNMHDR)lParam)->code) {
            case TVN_KEYDOWN: // 处理键盘事件
            {
                WPARAM wVKey = ((NMTVKEYDOWN*)lParam)->wVKey;
                if (wVKey == VK_SPACE)
                    HandleSpaceKey(hwnd);
                else
                    HandleHotkey(hwnd, wVKey);
                return 0;
            }
            case NM_DBLCLK: // 处理双击
            case NM_RETURN: // 处理回车键
            {
                HandleSpaceKey(hwnd);
                return 0;
            }
            break;
            }
        return 0;

    case WM_CLOSE:
        CleanupTree();
    case WM_DESTROY:
        CleanupTree();
        return 0;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
            CleanupTree();
        else {
            // 选中第一个项
            HTREEITEM hFirstItem = TreeView_GetRoot(g_hTreeView);
            if (hFirstItem)
                TreeView_SelectItem(g_hTreeView, hFirstItem);
            SetFocus(g_hTreeView);
        }
        return 0;
    case WM_NCPAINT: {
        if (g_currentTheme == ThemeMode::Dark) {
            HDC hdc = GetWindowDC(hwnd);
            RECT rc;
            GetWindowRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(32, 32, 32));
            FrameRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            ReleaseDC(hwnd, hdc);
            return 0;
        }
        break;
    }

    case WM_ERASEBKGND: {
        if (g_currentTheme == ThemeMode::Dark) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(32, 32, 32));
            FillRect((HDC)wParam, &rc, hBrush);
            HBRUSH hBorderBrush = CreateSolidBrush(RGB(64, 64, 64));
            FrameRect((HDC)wParam, &rc, hBorderBrush);
            DeleteObject(hBrush);
            DeleteObject(hBorderBrush);
        }
        else break;
        return 1;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 清理资源
void WINAPI CleanupTree() {
    if (g_treeItems) {
        FreeWindowList(g_treeItems, g_treeItemCount);
        g_treeItems = nullptr;
        g_treeItemCount = 0;
    }
    ShowWindow(g_hTreeWindow, SW_HIDE);
    showingTree = false;
}

// 创建树状视图
BOOL TreeInit(HINSTANCE hInstance) {
    // 如果窗口已存在，则直接返回
    if (g_hTreeWindow && IsWindow(g_hTreeWindow))
        return TRUE;

    // 注册窗口类
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = TreeWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TREE_WINDOW_CLASS;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
        return FALSE;

    // 创建窗口
    g_hTreeWindow = CreateWindowExW(
        0,
        TREE_WINDOW_CLASS, L"窗口列表",
        WS_POPUP | WS_BORDER ,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        NULL, NULL, hInstance, NULL
    );


    HWND hwndPrev = GetForegroundWindow();
    ActivateWindow(g_hTreeWindow);
    ActivateWindow(hwndPrev);

    return (g_hTreeWindow != NULL);
}

// 填充树状视图
void PopulateTreeView(UserItemInfo* treeItems, int treeItemCount) {
    if (!g_hTreeView || !treeItems)
        return;

    // 清空树状视图
    TreeView_DeleteAllItems(g_hTreeView);

    // 获取 DPI 并计算图标大小
    int dpi = CGetDpiForWindow(g_hTreeView);
    int iconSize = ScaleForDpi(32, dpi);

    // 创建适应 DPI 的图像列表
    HIMAGELIST hImageList = ImageList_Create(
        iconSize, iconSize,
        ILC_COLOR32 | ILC_MASK,
        treeItemCount + 1, 0
    );
    TreeView_SetImageList(g_hTreeView, hImageList, TVSIL_NORMAL);

    // 添加默认图标
    HICON hDefaultIcon = LoadIconW(NULL, IDI_APPLICATION);
    COLORREF bgColor = (g_currentTheme == ThemeMode::Dark) ? RGB(32, 32, 32) : RGB(255, 255, 255); // 亮色主题使用白色背景
    HBITMAP hBitmap = CreateMenuIconBitmap(hDefaultIcon, bgColor);
    if (hBitmap) {
        ImageList_Add(hImageList, hBitmap, NULL);
        DeleteObject(hBitmap);
    }
    else
        ImageList_AddIcon(hImageList, hDefaultIcon);


    // 为每个应用程序添加项目
    for (int i = 0; i < treeItemCount; i++) {
        const UserItemInfo& item = treeItems[i];

        // 添加应用程序图标到图像列表
        int imageIndex = 0;  // 默认图标索引
        if (item.windowCount > 0 && item.windows[0].hIcon) {
            HBITMAP hBitmap = CreateMenuIconBitmap(item.windows[0].hIcon, bgColor);
            if (hBitmap) {
                imageIndex = ImageList_Add(hImageList, hBitmap, NULL);
                DeleteObject(hBitmap);
            }
            else
                imageIndex = ImageList_AddIcon(hImageList, item.windows[0].hIcon);
        }

        // 创建带 hotkey 的父节点标题
        wchar_t parentTitle[MAX_TITLE_LENGTH + 2]; // +2 为 hotkey 和空格
        swprintf_s(parentTitle, L"%c %s", item.hotkey, item.name);

        // 创建父节点
        TVINSERTSTRUCT tvInsert = { 0 };
        tvInsert.hParent = TVI_ROOT;
        tvInsert.hInsertAfter = TVI_LAST;
        tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tvInsert.item.pszText = (LPWSTR)ProcTitle(parentTitle);  // 使用带 hotkey 的标题
        tvInsert.item.iImage = imageIndex;
        tvInsert.item.iSelectedImage = imageIndex;

        // 如果只有一个窗口，将窗口句柄存储在父节点
        if (item.windowCount == 1)
            tvInsert.item.lParam = (LPARAM)item.windows[0].hWnd;
        else
            tvInsert.item.lParam = 0;  // 多窗口时父节点不关联窗口句柄

        HTREEITEM hParent = TreeView_InsertItem(g_hTreeView, &tvInsert);

        // 如果有多个窗口，添加子节点
        if (item.windowCount > 1) {
            for (int j = 0; j < item.windowCount; j++) {
                const UserWinInfo& win = item.windows[j];

                // 创建带 hotkey 的子节点标题
                wchar_t childTitle[MAX_TITLE_LENGTH + 2]; // +2 为 hotkey 和空格
                swprintf_s(childTitle, L"%c %s", item.hotkey, win.winTitle);

                // 为每个窗口添加图标到图像列表
                int winImageIndex = 0;
                if (win.hIcon) {
                    HBITMAP hBitmap = CreateMenuIconBitmap(win.hIcon, bgColor);
                    if (hBitmap) {
                        winImageIndex = ImageList_Add(hImageList, hBitmap, NULL);
                        DeleteObject(hBitmap);
                    }
                    else
                        winImageIndex = ImageList_AddIcon(hImageList, win.hIcon);
                }

                // 创建子节点
                TVINSERTSTRUCT childInsert = { 0 };
                childInsert.hParent = hParent;
                childInsert.hInsertAfter = TVI_LAST;
                childInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                childInsert.item.pszText = (LPWSTR)childTitle;  // 使用带 hotkey 的标题
                childInsert.item.iImage = winImageIndex;
                childInsert.item.iSelectedImage = winImageIndex;
                childInsert.item.lParam = (LPARAM)win.hWnd;

                TreeView_InsertItem(g_hTreeView, &childInsert);
            }
        }
    }
}

// 设置窗口位置
void SetTreeWindowPos(POINT pt, bool followMouse) {
    if (!g_hTreeWindow)
        return;

    if (followMouse) {
        // 获取屏幕尺寸
        HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(MONITORINFO) };
        GetMonitorInfo(hMonitor, &mi);
        RECT screenRect = mi.rcWork;

        // 获取窗口尺寸
        RECT windowRect;
        GetWindowRect(g_hTreeWindow, &windowRect);
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;

        // 计算屏幕两侧可用空间
        int leftSpace = pt.x - screenRect.left;
        int rightSpace = screenRect.right - pt.x;

        // 根据可用空间决定窗口位置
        if (rightSpace > leftSpace) // 显示在右侧
            pt.x = min(pt.x, screenRect.right - windowWidth);
        else // 显示在左侧
            pt.x = max(pt.x - windowWidth, screenRect.left);
        // 确保窗口不会超出屏幕底部
        pt.y = min(pt.y, screenRect.bottom - windowHeight);

        SetWindowPos(g_hTreeWindow, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    }
    else {
        // 获取显示器中心点
        POINT centerPt = GetMonitorCenterPoint(pt);

        // 获取窗口大小
        RECT rc;
        GetWindowRect(g_hTreeWindow, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        // 设置窗口位置为屏幕中心
        SetWindowPos(g_hTreeWindow, NULL,
            centerPt.x - width / 2,
            centerPt.y - height / 2,
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    ActivateWindow(g_hTreeWindow);
}

// 修改 SetTreeTheme 函数
void SetTreeTheme(ThemeMode mode) {
    g_currentTheme = mode;
    if (!g_hTreeView) return;

    if (mode == ThemeMode::Dark) {
        // 设置树状视图主题
        SetWindowTheme(g_hTreeView, L"DarkMode_Explorer", nullptr);

        // 设置背景颜色
        TreeView_SetBkColor(g_hTreeView, RGB(32, 32, 32));
        TreeView_SetTextColor(g_hTreeView, RGB(240, 240, 240));
    } else {
        // 恢复默认主题
        SetWindowTheme(g_hTreeView, L"Explorer", nullptr);
        TreeView_SetBkColor(g_hTreeView, RGB(255, 255, 255)); // 亮色主题使用白色背景
        TreeView_SetTextColor(g_hTreeView, RGB(0, 0, 0)); // 亮色主题使用黑色文字
    }
}

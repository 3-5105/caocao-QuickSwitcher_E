struct UserItemInfo {
    wchar_t title[MAX_TITLE_LENGTH];
    wchar_t processName[MAX_PROCESS_NAME_LENGTH];
    int groupId;
    HWND hwnd;
    bool isMinimized;
    // 鍏朵粬闇€瑕佺殑瀛楁...
}; 

#include "window_utils.h"
#include <iostream>
#include <spdlog/spdlog.h>

/* Filter windows with these classnames */
const wchar_t* filter_classnames[] = {
    L"Windows.UI.Core.CoreWindow",
    L"Progman",
    L"WorkerW",
    L"ThumbnailDeviceHelperWnd",
    L"DummyDWMListenerWindow",
    L"ApplicationFrameWindow",
    L"EdgeUiInputTopWndClass",
    L"Shell_TrayWnd"
};

/* Get window title */
std::wstring get_window_title(HWND handle) {
    int length = GetWindowTextLengthW(handle);
    std::wstring ret;
    ret.resize(length+1);

    GetWindowTextW(handle, WW(ret), length+1);
    return ret;
}

/* Get window class name */
std::wstring get_window_class_name(HWND handle) {
    std::wstring ret;
    ret.resize(CLASSNAME_MAX_LENGTH);

    GetClassNameW(handle, WW(ret), CLASSNAME_MAX_LENGTH);

    ret.resize(1+wcslen(WW(ret)));
    return ret;
}

/* Filter invisible/hidden windows */
bool filter_window(HWND handle) {
    /* filter by basic visibility checks */
    if (!IsWindowVisible(handle) || !IsWindowEnabled(handle) || handle == GetConsoleWindow())
        return true;

    WINDOWINFO wi{};
    GetWindowInfo(handle, &wi);

    /* filter windows with less area */
    RECT r = wi.rcWindow;
    long long area = (r.bottom - r.top) * (r.right - r.left);
    if (area <= MIN_WINDOW_AREA)
        return true;

    /* filter windows with classnames */
    std::wstring class_name = get_window_class_name(handle);
    for (auto bl: filter_classnames) {
        if (class_name.find(bl) != -1) {
            return true;
        }
    }
    
    /* child windows would always have a parent? */
    if (wi.dwStyle & WS_CHILD || wi.dwStyle & WS_DISABLED) {
        return true;
    }

    return false;
}

/* Open process handle */
HANDLE get_process_handle(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
    if (hProcess == INVALID_HANDLE_VALUE) {
        spdlog::error("{}: cannot get process handle for pid={}", __func__, pid);
        return {};
    }
    return hProcess;
}

/* Get image path from window handle */
std::wstring get_process_name(HANDLE hProcess) {
    if (!hProcess) {
        spdlog::error("{}: invalid process handle.", __func__);
        return L"";
    }

    std::wstring ret;
    ret.resize(PROCESSNAME_MAX_LENGTH+1);

    DWORD recv{PROCESSNAME_MAX_LENGTH};
    BOOL result = QueryFullProcessImageNameW(hProcess, 0, WW(ret), &recv);
    if (!result) {
        spdlog::error("{}: querying process name failed (err={})", __func__, GetLastError());
        return L"";
    }

    ret.resize(1+recv);
    return ret;
}

/* EnumWindows callback */
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lparam) {
    if (filter_window(handle))
        return true;

    DWORD pid{};
    GetWindowThreadProcessId(handle, &pid);

    HANDLE hProcess = get_process_handle(pid);
    if (!hProcess)
        return true;

    auto ret = reinterpret_cast<std::map<HANDLE, window_info>*>(lparam);
    window_info wi = {
        .title = get_window_title(handle),
        .path = get_process_name(hProcess),
        .pid = pid
    };

    CloseHandle(handle);
    ret->emplace(handle, wi);
    return true;
}

namespace window_utils {
    std::map<HANDLE, window_info> find_active_windows() {
        std::map<HANDLE, window_info> ret;
        EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&ret));
        return ret;
    }
};
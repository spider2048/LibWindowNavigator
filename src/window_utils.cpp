#include "window_utils.h"
#include <iostream>
#include <search.h>
#include <spdlog/spdlog.h>

/* Filter windows with these classnames */
const char* filter_classnames[] = {
    "Windows.UI.Core.CoreWindow",
    "Progman",
    "WorkerW",
    "ThumbnailDeviceHelperWnd",
    "DummyDWMListenerWindow",
    "ApplicationFrameWindow",
    "EdgeUiInputTopWndClass",
    "Shell_TrayWnd",
    "NarratorHelperWindow"
};

namespace window_utils {
/* Get window title */
std::string get_window_title(HWND handle) {
    int length = GetWindowTextLengthA(handle);
    std::string ret;
    ret.resize(length+1);

    GetWindowTextA(handle, WW(ret), length+1);
    return ret;
}

/* Get window class name */
std::string get_window_class_name(HWND handle) {
    std::string ret;
    ret.resize(CLASSNAME_MAX_LENGTH);

    GetClassNameA(handle, WW(ret), CLASSNAME_MAX_LENGTH);

    ret.resize(1+strlen(WW(ret)));
    spdlog::debug("get_window_class_name: {}", ret);
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
    std::string class_name = get_window_class_name(handle);
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
std::string get_process_name(HANDLE hProcess) {
    if (!hProcess) {
        spdlog::error("{}: invalid process handle.", __func__);
        return "";
    }

    std::string ret;
    ret.resize(PROCESSNAME_MAX_LENGTH+1);

    DWORD recv{PROCESSNAME_MAX_LENGTH};
    BOOL result = QueryFullProcessImageNameA(hProcess, 0, WW(ret), &recv);
    if (!result) {
        spdlog::error("{}: querying process name failed (err={})", __func__, GetLastError());
        return "";
    }

    ret.resize(1+recv);
    return ret;
}
};

/* EnumWindows callback */
BOOL CALLBACK enum_windows_callback(HWND window_handle, LPARAM lparam) {
    auto manager = reinterpret_cast<window_manager*>(lparam);

    if (window_utils::filter_window(window_handle)){
        spdlog::debug("filtering window {}", (void*) window_handle);
        return true;
    }

    DWORD pid{};
    GetWindowThreadProcessId(window_handle, &pid);
    if (!pid) {
        spdlog::error("getting pid of {} failed", (void*) window_handle);
        return true;
    }

    if (manager->is_cached(window_handle, pid)){
        spdlog::debug("window_handle={} pid={} is cached", (void*) window_handle, pid);
        return true;
    }

    HANDLE hProcess = window_utils::get_process_handle(pid);
    if (!hProcess) {
        spdlog::error("getting handle for pid {} failed", pid);
        return true;
    }

    std::shared_ptr<window_info> wi = std::make_shared<window_info>();
    wi->title = window_utils::get_window_title(window_handle);
    wi->path = window_utils::get_process_name(hProcess);
    wi->pid = pid;

    CloseHandle(hProcess);

    manager->add_window(window_handle, wi);
    return true;
}

void window_manager::add_window(HWND window_handle, const std::shared_ptr<window_info>& wi) {
    spdlog::debug("adding window_handle={} pid={}", (void*)window_handle, wi->pid);
    windows.emplace(window_handle, wi);
}

void window_manager::find_active_windows() {
    refresh_cache();
    EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(this));
}

bool window_manager::is_cached(HWND window_handle, DWORD pid) {
    if (windows.count(window_handle))
        return windows[window_handle]->pid == pid;
    return false;
}

void window_manager::remove_window(HWND window_handle) {
    const auto it = windows.find(window_handle);
    if (it != windows.end()) {
        spdlog::debug("removing non-existent window {} pid={}", (void*) window_handle, it->second->pid);
        windows.erase(it);
    } else {
        spdlog::debug("illegal call to remove_window(...) with window_handle={}", (void*) window_handle);
    }
}

void window_manager::refresh_cache() {
    spdlog::debug("refreshing cache");

    std::vector<HWND> marked_for_deletion;

    for (auto& [hwnd, wi]: windows) {
        DWORD pid = wi->pid;
        DWORD pid2{};

        if (GetWindowThreadProcessId(hwnd, &pid2) == 0 || pid != pid2){
            marked_for_deletion.push_back(hwnd);
        }
    }

    spdlog::debug("marked {} windows for deletion", marked_for_deletion.size());
    for (auto hwnd: marked_for_deletion) {
        remove_window(hwnd);
    }
}

std::vector<std::shared_ptr<window_info>> window_manager::rank_results(const std::string& target, int n) {
    std::map<HWND, double> scores_title, scores_path;

    for (auto& [h, wi] : windows) {
        scores_title.emplace(
            h,
            search::get_score(target, wi->title)
        );

        scores_path.emplace(
            h,
            search::get_score(target, wi->path)
        );
    }

    std::vector<HWND> ret;

    for (auto& [key, _] : windows) {
        ret.push_back(key);
    }

    std::sort(ret.begin(), ret.end(), [&](HWND s1, HWND s2) {
        double score1 = 0.9 * scores_title[s1] + 0.1 * scores_path[s1];
        double score2 = 0.9 * scores_title[s2] + 0.1 * scores_path[s2];
        return score1 > score2;
    });


    std::vector<std::shared_ptr<window_info>> retw;
    for (int i=0; i<std::min<size_t>(n, ret.size()); ++i) {
        retw.push_back(windows[ret[i]]);
    }

    return retw;
}

void window_manager::set_active_window(HWND hwnd) {
    // Show new window
    WINDOWPLACEMENT wp{};
    
    // Hide active window
    HWND foreground = GetForegroundWindow();
    GetWindowPlacement(foreground, &wp);

    switch (wp.showCmd) {
    case SW_MAXIMIZE:
    case SW_MAX:
        ShowWindow(foreground, SW_RESTORE);
    }

    GetWindowPlacement(hwnd, &wp);
    
    switch (wp.showCmd) {
    case SW_SHOWMINIMIZED:
        ShowWindow(hwnd, SW_RESTORE);
        break;
    case SW_HIDE:
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        break;
    }

    SetActiveWindow(hwnd);
    SetForegroundWindow(hwnd);
}
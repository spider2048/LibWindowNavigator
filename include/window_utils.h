#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#define CLASSNAME_MAX_LENGTH 512
#define PROCESSNAME_MAX_LENGTH 512
#define MIN_WINDOW_AREA 10
#define WW(X) const_cast<char*>(X.c_str())

class window_info {
public:
    std::string title;
    std::string path;
    DWORD pid;
};

typedef std::unordered_map<HWND, std::shared_ptr<window_info>> window_collection_t;

class window_manager {
public:
    window_collection_t windows;

    void find_active_windows();
    void add_window(HWND window_handle, const std::shared_ptr<window_info>& wi);
    void remove_window(HWND window_handle);
    bool is_cached(HWND window_handle, DWORD pid);
    void refresh_cache();

    void set_active_window(HWND window_handle);

    std::vector<std::shared_ptr<window_info>> rank_results(const std::string& target, int n);
};
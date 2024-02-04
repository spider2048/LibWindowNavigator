#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#define CLASSNAME_MAX_LENGTH 512
#define PROCESSNAME_MAX_LENGTH 512
#define MIN_WINDOW_AREA 10
#define WW(X) const_cast<wchar_t*>(X.c_str())

typedef struct {
    std::wstring title;
    std::wstring path;
    DWORD pid;
} window_info;

namespace window_utils {
    std::map<HANDLE, window_info> find_active_windows();
};
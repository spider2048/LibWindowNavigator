#pragma once
#define NOMINMAX

#include <string>
#include <vector>
#include <Windows.h>
#include <window_utils.h>

namespace search {
    double get_score(const std::string& str1, const std::string& str2);
};
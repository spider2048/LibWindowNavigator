#include "search.h"
#include <rapidfuzz/fuzz.hpp>

namespace search {
double get_score(const std::string& str1, const std::string& str2) {
    return rapidfuzz::fuzz::token_sort_ratio(str1, str2);
}

void rank_results(const std::string& target, std::vector<std::string>& results) {
    std::sort(results.begin(), results.end(), [&](const std::string& str1, const std::string& str2) {
        return get_score(target, str1) > get_score(target, str2);
    });
}
};
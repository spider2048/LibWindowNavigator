#define NOMINMAX /* Windows API */

#include <Windows.h>
#include <spdlog/spdlog.h>

#include <gui.h>
#include <window_utils.h>
#include <utils.h>

int main() {
    spdlog::set_level(spdlog::level::debug);

    // auto list = window_utils::find_active_windows();
    // for (auto& wi: list) {
    //     auto w = wi.second;
    //     spdlog::info("pid={} path={} title={}", w.pid, to_string(w.path), to_string(w.title));
    // }

    // gui g;
    // g.loop();
    // return 0;
}   
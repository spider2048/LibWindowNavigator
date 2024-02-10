#define NOMINMAX /* Windows API */
#define SDL_

#include <Windows.h>
#include <spdlog/spdlog.h>

#include <gui.h>
#include <search.h>
#include <window_utils.h>
#include <utils.h>

#include <iostream>

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    // window_manager wm;
    // spdlog::info("checking available windows");
    // wm.find_active_windows();
    
    // spdlog::info("found {} windows", wm.windows.size());
    
    // while (true) {
    //     std::string buf;

    //     std::cout << "$ ";
    //     std::cin >> buf;

    //     std::vector<HWND> results = wm.rank_results(buf);
    //     for (int i=0; i<std::min<int>(results.size(), 10); ++i) {
    //         auto p = wm.windows[results[i]];
    //         std::cout << p->pid << " | " << p->path << " | " << p->title << std::endl;
    //     }
    // }

    gui g;
    g.loop();

    return 0;
}
#include "gui.h"
#include <spdlog/spdlog.h>

gui::gui() {
    register_hotkey();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("cannot initialize SDL");
        spdlog::error("SDL_Error: {}", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(
        "WindowNavigator", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        400,
        0
    );

    if (window == nullptr) {
        spdlog::error("cannot create window");
        spdlog::error("SDL_Error: {}", SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_SOFTWARE
    );

    if (renderer == nullptr) {
        spdlog::error("cannot create window");
        spdlog::error("SDL_Error: {}", SDL_GetError());
        return;
    }

    if (TTF_Init() < 0) {
        spdlog::error("cannot init TTF");
        spdlog::error("SDL_Error: {}", SDL_GetError());
        return;
    }

    font = TTF_OpenFont("C:\\Windows\\Fonts\\calibri.ttf", FontSize);
    if (font == nullptr) {
        spdlog::error("failed to open font");
        spdlog::error("SDL_Error: {}", SDL_GetError());
        return;
    }
}

void gui::free_resources() {
    for (int i=0; i<surfaces.size(); ++i)
        SDL_FreeSurface(surfaces[i]);
    
    for (int i=0; i<textures.size(); ++i)
        SDL_DestroyTexture(textures[i]);
}

gui::~gui() {
    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    TTF_CloseFont(font);

    TTF_Quit();
    SDL_Quit();
    unregister_hotkey();
}

void gui::loop() {
    pre_loop();
    while (running) {
        auto start = SDL_GetTicks();

        poll_events();
        inner_loop();

        auto frame = SDL_GetTicks() - start;
        if (frame < FRAME_DELAY)
            SDL_Delay(FRAME_DELAY - frame);
    }
}

void gui::register_hotkey() {
    if (!RegisterHotKey(nullptr, HOTKEY_ID, MOD_NOREPEAT | MOD_WIN, VK_F2)) {
        spdlog::error("Hotkey registration failed");
        return;
    }
}

void gui::unregister_hotkey() {
    UnregisterHotKey(NULL, HOTKEY_ID);
}

void gui::poll_events() {
    /* Windows events */
    MSG msg;
    while (PeekMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY, PM_REMOVE)) {
        if (msg.wParam == HOTKEY_ID) {
            toggle_window();
        }
    }

    /* SDL events */
    SDL_Event ev{};
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE) {
            running = false;
        }

        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE) {
                hide_window();
            } else if (ev.key.keysym.sym == SDLK_DOWN) {
                selected += 1;
                selected = std::min<size_t>(selected, ranks.size());
            } else if (ev.key.keysym.sym == SDLK_UP) {
                selected -= 1;
                selected = std::max<size_t>(selected, ranks.size());
            }
        }
    }
}

void gui::draw_option(const std::shared_ptr<window_info>& wi, int y) {
    std::string display = wi->title + " ~ " + wi->path;

    SDL_Surface* surface = TTF_RenderText_Solid(font, display.c_str(), TextColor);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    textures.push_back(texture);
    surfaces.push_back(surface);
}

void gui::refresh() {
    SDL_SetRenderDrawColor(renderer, BACKGROUND);
    SDL_RenderClear(renderer);

    free_resources();

    wm.find_active_windows();
    auto ranks = wm.rank_results("code", 10);

    draw_option(ranks[0], 10);
}

void gui::pre_loop() {
    refresh();
}

void gui::inner_loop() {
    // refresh();
}

void gui::hide_window() {
    SDL_HideWindow(window);
    window_state = window_states::hidden;
}

void gui::show_window() {
    SDL_ShowWindow(window);
    window_state = window_states::shown;
}

void gui::toggle_window() {
    switch (window_state) {
        case window_states::shown:
            hide_window();
            break;
        case window_states::hidden:
            show_window();
            break;
    }

    refresh();
}

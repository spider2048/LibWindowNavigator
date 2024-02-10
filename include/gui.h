#pragma once
#define SDL_MAIN_HANDLED

#include <vector>
#include <spdlog/fmt/fmt.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <window_utils.h>

#define PANIC(...) throw std::runtime_error(fmt::format(__VA_ARGS__));

constexpr int FRAME_DELAY = 1000 / 30;
constexpr int HOTKEY_ID = 10;

enum class window_states {
    shown, hidden
};

#define BACKGROUND 0xFD, 0xBF, 0x60, 0x00

class gui {
    bool running = true;
    window_states window_state = window_states::shown;

    SDL_Color Background = {0xFD, 0xBF, 0x60, 0x00}, 
              Selected   = {0x9F, 0x70, 0xFD, 0x00},
              TextColor  = {0, 0, 0, 0};

    int Width = 800, Height = 400, xPad = 10, yPad = 10, ResultHeight = 50;
    int FontSize = 10;

    SDL_Window* window{};
    SDL_Renderer* renderer{};

    window_manager wm;
    std::vector<std::shared_ptr<window_info>> ranks;
    int selected = 0;

    std::vector<SDL_Texture*> textures;
    std::vector<SDL_Surface*> surfaces;

    TTF_Font* font{};

    void pre_loop();
    void inner_loop();
    void poll_events();

    void draw_option(const std::shared_ptr<window_info>& wi, int y);
    void refresh();

    void free_resources();
    void register_hotkey();
    void unregister_hotkey();
public:
    gui();
    void loop();
    ~gui();

    void hide_window();
    void show_window();
    void toggle_window();
};
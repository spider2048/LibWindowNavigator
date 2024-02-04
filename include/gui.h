#include <spdlog/fmt/fmt.h>

#define PANIC(...) throw std::runtime_error(fmt::format(__VA_ARGS__));

class gui {
    bool running = true;

    void pre_loop();
    void inner_loop();
    void poll_events();
public:
    gui();
    void loop();
    ~gui();
};
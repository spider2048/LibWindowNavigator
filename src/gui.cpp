#include "gui.h"

gui::gui() {

}

gui::~gui() {

}

void gui::loop() {
    pre_loop();
    while (running) {
        poll_events();
        inner_loop();
    }
}

void gui::poll_events() {

}

void gui::pre_loop() {

}

void gui::inner_loop() {

}
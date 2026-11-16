#pragma once
#include <string>

// Callbacks pune ovu strukturu, App::update() je čita, endFrame() briše jednokratne događaje.
struct Input {
    float mx = -100.f, my = -100.f;   // pozicija miša u referentnim koordinatama
    bool lmbDown = false, lmbPressed = false, lmbReleased = false;
    bool backspace = false, enter = false;
    std::string typed;                // znakovi iz char callback-a (cifre, '.')

    void endFrame() {
        lmbPressed = lmbReleased = backspace = enter = false;
        typed.clear();
    }
};

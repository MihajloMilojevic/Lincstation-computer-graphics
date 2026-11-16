#pragma once

struct Vec2 { float x, y; };

// Pravougaonik u referentnim koordinatama (960x540), (x,y) = gornji levi ugao.
struct Rect {
    float x, y, w, h;
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
    Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }
};

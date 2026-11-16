#pragma once
#include <algorithm>

// Kretanje konstantnom brzinom ka cilju (brzina u jedinicama u sekundi).
struct Anim {
    float v, target, speed;
    void update(float dt) {
        float d = speed * dt;
        v = (v < target) ? std::min(target, v + d) : std::max(target, v - d);
    }
};

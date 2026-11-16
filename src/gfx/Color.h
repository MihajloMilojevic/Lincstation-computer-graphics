#pragma once
#include <glm/glm.hpp>

// 0xRRGGBB -> vec4
inline glm::vec4 hx(unsigned c, float a = 1.f) {
    return { ((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, a };
}

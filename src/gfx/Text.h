#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Renderer.h"

// Tekst preko FreeType-a. Glifovi se rasterizuju u veličini koja odgovara stvarnom ekranu
// (size * scale renderera), keširaju se kao R8 teksture i crtaju kao quadovi (mode 2).
class Text {
public:
    enum class Align { Left, Center, Right };

    explicit Text(Renderer& r) : r_(r) {}
    ~Text();
    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;

    bool init(const std::string& fontPath);

    // (x, baselineY) u referentnim koordinatama, size = visina fonta u referentnim jedinicama
    void draw(const std::string& utf8, float x, float baselineY, float size, const glm::vec4& color,
              Align align = Align::Left, float alpha = 1.f);
    float width(const std::string& utf8, float size);

private:
    struct Glyph { unsigned tex = 0; int w = 0, h = 0, left = 0, top = 0; float adv = 0.f; };
    const Glyph& glyph(int px, char32_t cp);
    int pixelSize(float size) const;

    Renderer& r_;
    FT_Library lib_ = nullptr;
    FT_Face face_ = nullptr;
    std::unordered_map<unsigned long long, Glyph> cache_;
};

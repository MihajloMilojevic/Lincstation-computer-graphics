#include "Text.h"
#include <glad/gl.h>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
std::vector<char32_t> decodeUtf8(const std::string& s) {
    std::vector<char32_t> out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        char32_t cp = 0;
        int n = 0;
        if (c < 0x80)           { cp = c;        n = 1; }
        else if ((c >> 5) == 6) { cp = c & 0x1F; n = 2; }
        else if ((c >> 4) == 14){ cp = c & 0x0F; n = 3; }
        else if ((c >> 3) == 30){ cp = c & 0x07; n = 4; }
        else { ++i; continue; }
        for (int k = 1; k < n && i + k < s.size(); ++k) cp = (cp << 6) | (s[i + k] & 0x3F);
        out.push_back(cp);
        i += n;
    }
    return out;
}
}

Text::~Text() {
    for (auto& kv : cache_)
        if (kv.second.tex) glDeleteTextures(1, &kv.second.tex);
    if (face_) FT_Done_Face(face_);
    if (lib_) FT_Done_FreeType(lib_);
}

bool Text::init(const std::string& fontPath) {
    if (FT_Init_FreeType(&lib_)) { std::cerr << "FreeType init failed\n"; return false; }
    if (FT_New_Face(lib_, fontPath.c_str(), 0, &face_)) {
        std::cerr << "Cannot load font: " << fontPath << "\n";
        return false;
    }
    return true;
}

int Text::pixelSize(float size) const {
    return std::max(6, (int)std::lround(size * r_.scale()));
}

const Text::Glyph& Text::glyph(int px, char32_t cp) {
    unsigned long long key = ((unsigned long long)px << 32) | (unsigned long long)cp;
    auto it = cache_.find(key);
    if (it != cache_.end()) return it->second;

    Glyph g;
    FT_Set_Pixel_Sizes(face_, 0, px);
    if (FT_Load_Char(face_, cp, FT_LOAD_RENDER) == 0) {
        FT_GlyphSlot s = face_->glyph;
        g.w = (int)s->bitmap.width;
        g.h = (int)s->bitmap.rows;
        g.left = s->bitmap_left;
        g.top = s->bitmap_top;
        g.adv = (float)(s->advance.x >> 6);
        if (g.w > 0 && g.h > 0) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glGenTextures(1, &g.tex);
            glBindTexture(GL_TEXTURE_2D, g.tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, g.w, g.h, 0, GL_RED, GL_UNSIGNED_BYTE, s->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    return cache_.emplace(key, g).first->second;
}

float Text::width(const std::string& utf8, float size) {
    int px = pixelSize(size);
    float w = 0.f;
    for (char32_t cp : decodeUtf8(utf8)) w += glyph(px, cp).adv;
    return w / r_.scale();
}

void Text::draw(const std::string& utf8, float x, float baselineY, float size,
                const glm::vec4& color, Align align, float alpha) {
    const float sc = r_.scale();
    const int px = pixelSize(size);
    float w = width(utf8, size);
    if (align == Align::Center) x -= w * 0.5f;
    else if (align == Align::Right) x -= w;

    // pen u uređajskim pikselima, poravnat na ceo piksel radi oštrine
    float pen = std::round(x * sc);
    const float by = std::round(baselineY * sc);
    for (char32_t cp : decodeUtf8(utf8)) {
        const Glyph& g = glyph(px, cp);
        if (g.tex) {
            Rect q{ (pen + g.left) / sc, (by - g.top) / sc, g.w / sc, g.h / sc };
            r_.drawGlyph(g.tex, q, color, alpha);
        }
        pen += g.adv;
    }
}

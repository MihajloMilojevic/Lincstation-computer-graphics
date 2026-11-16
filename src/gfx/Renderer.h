#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Texture.h"
#include "core/Rect.h"

// Crta sve preko jednog jediničnog kvadrata (VAO/VBO se prave jednom).
// Svi pozivi koriste referentne koordinate 960x540; Renderer ih skalira na ekran
// (uz letterbox ako monitor nije 16:9). Oblici (zaobljeni pravougaonik, elipsa)
// se crtaju u fragment šejderu preko SDF-a, bez dodatnih tekstura.
class Renderer {
public:
    static constexpr float REF_W = 960.f, REF_H = 540.f;

    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(int winW, int winH, int fbW, int fbH);
    void beginFrame(const glm::vec3& clearColor);

    void drawTex(const Texture& t, Rect r, float alpha = 1.f, bool flipX = false,
                 const glm::vec4& tint = glm::vec4(1.f));
    void drawRect(Rect r, const glm::vec4& color, float alpha = 1.f);
    void drawRound(Rect r, const glm::vec4& color, float radius, float alpha = 1.f);
    void drawEllipse(Rect r, const glm::vec4& color, float alpha = 1.f);
    void drawOutlined(Rect r, const glm::vec4& fill, const glm::vec4& stroke, float radius,
                      float width = 1.f, float alpha = 1.f);
    void drawEllipseOutlined(Rect r, const glm::vec4& fill, const glm::vec4& stroke,
                             float width = 1.f, float alpha = 1.f);
    void drawStroke(Rect r, const glm::vec4& color, float width, float alpha = 1.f);
    void drawDashed(Rect r, const glm::vec4& color, float dash, float gap, float width = 1.f);
    void drawGlyph(unsigned glTex, Rect r, const glm::vec4& color, float alpha = 1.f);

    // y' = originY + (y - originY) * scaleY + dy  (preklapanje poklopca/vrata, izvlačenje fioke)
    void setXform(float originY, float scaleY, float dy);
    void resetXform() { setXform(0.f, 1.f, 0.f); }

    // koordinate prozora (GLFW cursor pos) -> referentne koordinate
    glm::vec2 toRef(double sx, double sy) const;
    float scale() const { return scale_; }

private:
    void quad(Rect r, int mode, int shape, float radius, const glm::vec4& color, float alpha,
              bool flipX = false);

    Shader prog_;
    unsigned vao_ = 0, vbo_ = 0;
    int winW_ = 1, winH_ = 1, fbW_ = 1, fbH_ = 1;
    float scale_ = 1.f, vx_ = 0.f, vy_ = 0.f;
};

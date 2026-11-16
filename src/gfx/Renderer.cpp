#include "Renderer.h"
#include <glad/gl.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

Renderer::~Renderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

bool Renderer::init(int winW, int winH, int fbW, int fbH) {
    winW_ = std::max(winW, 1); winH_ = std::max(winH, 1);
    fbW_ = std::max(fbW, 1);   fbH_ = std::max(fbH, 1);

    scale_ = std::min(fbW_ / REF_W, fbH_ / REF_H);
    vx_ = (fbW_ - REF_W * scale_) * 0.5f;
    vy_ = (fbH_ - REF_H * scale_) * 0.5f;

    if (!prog_.load("shaders/quad.vert", "shaders/quad.frag")) return false;

    const float verts[] = { 0.f, 0.f,  1.f, 0.f,  0.f, 1.f,  1.f, 1.f };  // triangle strip
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    prog_.use();
    prog_.set("uProj", glm::ortho(0.f, REF_W, REF_H, 0.f));   // y raste nadole kao u prototipu
    prog_.set("uTexture", 0);
    resetXform();
    return true;
}

void Renderer::beginFrame(const glm::vec3& clearColor) {
    glViewport(0, 0, fbW_, fbH_);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport((int)vx_, (int)vy_, (int)(REF_W * scale_), (int)(REF_H * scale_));
    prog_.use();
    glBindVertexArray(vao_);
    resetXform();
}

void Renderer::setXform(float originY, float scaleY, float dy) {
    prog_.set("uXform", glm::vec3(originY, scaleY, dy));
}

void Renderer::quad(Rect r, int mode, int shape, float radius, const glm::vec4& color,
                    float alpha, bool flipX) {
    prog_.set("uMode", mode);
    prog_.set("uShape", shape);
    prog_.set("uRadius", radius);
    prog_.set("uPos", glm::vec2(r.x, r.y));
    prog_.set("uSize", glm::vec2(r.w, r.h));
    prog_.set("uFlipX", flipX);
    prog_.set("uColor", color);
    prog_.set("uAlpha", alpha);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::drawTex(const Texture& t, Rect r, float alpha, bool flipX, const glm::vec4& tint) {
    t.bind(0);
    quad(r, 1, 0, 0.f, tint, alpha, flipX);
}

void Renderer::drawGlyph(unsigned glTex, Rect r, const glm::vec4& color, float alpha) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glTex);
    quad(r, 2, 0, 0.f, color, alpha);
}

void Renderer::drawRect(Rect r, const glm::vec4& color, float alpha) {
    quad(r, 0, 0, 0.f, color, alpha);
}

void Renderer::drawRound(Rect r, const glm::vec4& color, float radius, float alpha) {
    quad(r, 0, 1, radius, color, alpha);
}

void Renderer::drawEllipse(Rect r, const glm::vec4& color, float alpha) {
    quad(r, 0, 2, 0.f, color, alpha);
}

void Renderer::drawOutlined(Rect r, const glm::vec4& fill, const glm::vec4& stroke, float radius,
                            float width, float alpha) {
    quad(r, 0, 1, radius, stroke, alpha);
    Rect in{r.x + width, r.y + width, r.w - 2 * width, r.h - 2 * width};
    quad(in, 0, 1, std::max(0.f, radius - width), fill, alpha);
}

void Renderer::drawEllipseOutlined(Rect r, const glm::vec4& fill, const glm::vec4& stroke,
                                   float width, float alpha) {
    quad(r, 0, 2, 0.f, stroke, alpha);
    quad({r.x + width, r.y + width, r.w - 2 * width, r.h - 2 * width}, 0, 2, 0.f, fill, alpha);
}

void Renderer::drawStroke(Rect r, const glm::vec4& color, float w, float alpha) {
    drawRect({r.x, r.y, r.w, w}, color, alpha);
    drawRect({r.x, r.y + r.h - w, r.w, w}, color, alpha);
    drawRect({r.x, r.y + w, w, r.h - 2 * w}, color, alpha);
    drawRect({r.x + r.w - w, r.y + w, w, r.h - 2 * w}, color, alpha);
}

void Renderer::drawDashed(Rect r, const glm::vec4& color, float dash, float gap, float w) {
    for (float x = r.x; x < r.x + r.w; x += dash + gap) {
        float len = std::min(dash, r.x + r.w - x);
        drawRect({x, r.y, len, w}, color);
        drawRect({x, r.y + r.h - w, len, w}, color);
    }
    for (float y = r.y; y < r.y + r.h; y += dash + gap) {
        float len = std::min(dash, r.y + r.h - y);
        drawRect({r.x, y, w, len}, color);
        drawRect({r.x + r.w - w, y, w, len}, color);
    }
}

glm::vec2 Renderer::toRef(double sx, double sy) const {
    float fx = (float)(sx * fbW_ / winW_);
    float fy = (float)(sy * fbH_ / winH_);
    return { (fx - vx_) / scale_, (fy - vy_) / scale_ };
}

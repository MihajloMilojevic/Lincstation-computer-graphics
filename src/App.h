#pragma once
#include "core/Input.h"
#include "gfx/Renderer.h"
#include "gfx/Text.h"
#include "gfx/Texture.h"
#include "model/Cashier.h"
#include "model/Machine.h"
#include "ui/DragSystem.h"

struct GLFWwindow;

// Kursor se bira kao čista funkcija stanja (mod, šta je ispod miša, da li se nešto vuče).
enum class CursorKind { Banknote, GreenBanknote, Coin, Padlock, Key, Hand, Count };

class App {
public:
    explicit App(GLFWwindow* win) : win_(win), text_(r_) {}
    bool init();

    // GLFW događaji (pozivaju se iz callback-a u main.cpp)
    void onKey(int key, int action);
    void onChar(unsigned codepoint);
    void onMouseMove(double x, double y);
    void onMouseButton(int button, int action);

    void update(float dt);
    void draw();

private:
    void onPress();
    CursorKind pickCursor() const;

    void drawModePill();
    void drawSettings();
    void drawCitZones();
    void drawCassettes();
    void drawCashier();
    void drawTray();
    void drawMoney(bool bill, int cents, Vec2 center, float alpha = 1.f);
    void drawNameOverlay();
    void drawCursor();

    GLFWwindow* win_;
    Renderer r_;
    Text text_;
    Input in_;
    Machine m_;
    Cashier cashier_;
    DragSystem drag_;
    Texture overlayTex_, cursorTex_[(int)CursorKind::Count];
    CursorKind cur_ = CursorKind::Banknote;
    bool labels_ = false;
    float time_ = 0.f;
};

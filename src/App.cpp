#include "App.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "gfx/Color.h"
#include "model/Money.h"
#include "ui/Layout.h"
#include "ui/MachineView.h"

namespace {
const glm::vec3 BG_COLOR{0.835f, 0.855f, 0.886f};     // #d5dae2

const char* CURSOR_FILES[(int)CursorKind::Count] = {
    "assets/tex/cursor_banknote.png", "assets/tex/cursor_banknote_green.png",
    "assets/tex/cursor_coin.png", "assets/tex/cursor_padlock.png",
    "assets/tex/cursor_key.png", "assets/tex/cursor_hand.png"};

glm::vec4 billColor(int v) {
    switch (v) { case 500: return hx(0x9aa09a); case 1000: return hx(0xc4504a); case 2000: return hx(0x4a78c4);
                 case 5000: return hx(0xd88a3a); default: return hx(0x5fa05a); }
}
glm::vec4 billText(int v) {
    switch (v) { case 500: return hx(0x2c2f2c); case 1000: case 2000: return hx(0xffffff);
                 case 5000: return hx(0x2c1a05); default: return hx(0x10240e); }
}
Rect casRect(Vec2 p) { return {p.x, p.y, L::CAS_W, L::CAS_H}; }
}

bool App::init() {
    int ww, wh, fw, fh;
    glfwGetWindowSize(win_, &ww, &wh);
    glfwGetFramebufferSize(win_, &fw, &fh);
    if (!r_.init(ww, wh, fw, fh)) return false;
    if (!text_.init("assets/fonts/DejaVuSans.ttf")) return false;
    if (!overlayTex_.load("assets/tex/name_overlay.png")) return false;
    for (int i = 0; i < (int)CursorKind::Count; ++i)
        if (!cursorTex_[i].load(CURSOR_FILES[i])) return false;
    return true;
}

// ---------------------------------------------------------------- događaji
void App::onKey(int key, int action) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(win_, GLFW_TRUE); return; }   // uvek, u svakom stanju
    if (key == GLFW_KEY_BACKSPACE) in_.backspace = true;
    if (action == GLFW_PRESS && (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)) in_.enter = true;
}

void App::onChar(unsigned c) {
    if (c == ',') c = '.';                                    // decimalni zarez sa srpske tastature
    if ((c >= '0' && c <= '9') || c == '.') in_.typed.push_back((char)c);
}

void App::onMouseMove(double x, double y) {
    glm::vec2 p = r_.toRef(x, y);
    in_.mx = p.x;
    in_.my = p.y;
}

void App::onMouseButton(int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_PRESS)   { in_.lmbDown = true;  in_.lmbPressed = true; }
    if (action == GLFW_RELEASE) { in_.lmbDown = false; in_.lmbReleased = true; }
}

// ---------------------------------------------------------------- logika
void App::update(float dt) {
    time_ += dt;
    m_.update(dt);
    cashier_.handleTyping(in_, m_);
    if (in_.lmbPressed) onPress();
    drag_.update(dt, in_, m_);
    cur_ = pickCursor();
    in_.endFrame();
}

void App::onPress() {
    const float x = in_.mx, y = in_.my;

    // 1) dugmad kasira (u CIT modu nisu klikabilna)
    if (L::CASHIER_PANEL.contains(x, y)) {
        for (Button& b : cashier_.buttons(m_))
            if (b.enabled && b.onClick && b.rect.contains(x, y)) { b.onClick(); break; }
        return;
    }
    // 2) podešavanja
    if (L::SETTINGS.contains(x, y)) { labels_ = !labels_; return; }

    // 3) traka sa novcem: početak prevlačenja
    for (int i = 0; i < (int)BILLS.size(); ++i) {
        Vec2 c = L::billHome(i);
        if (Rect{c.x - L::BILL_W / 2, c.y - L::BILL_H / 2, L::BILL_W, L::BILL_H}.contains(x, y)) {
            drag_.startMoney(true, BILLS[i], c, in_);
            return;
        }
    }
    for (int i = 0; i < (int)COINS.size(); ++i) {
        Vec2 c = L::coinHome(i);
        float dx = x - c.x, dy = y - c.y;
        if (dx * dx + dy * dy <= L::COIN_R * L::COIN_R) {
            drag_.startMoney(false, COINS[i], c, in_);
            return;
        }
    }

    // 4) CIT: kasete
    if (m_.canGrabCassette() && casRect(drag_.casPos).contains(x, y)) {
        drag_.startCassette(DragSystem::Kind::Cassette, in_);
        return;
    }
    if (m_.canGrabNewCassette() && casRect(drag_.ncasPos).contains(x, y)) {
        drag_.startCassette(DragSystem::Kind::NewCassette, in_);
        return;
    }

    // 5) mašina: otvor za kusur, vrata, poklopac, fioka
    if (m_.change && L::WINDOW.contains(x, y)) { m_.takeChange(); return; }
    if (L::DOOR.contains(x, y)) { m_.clickDoor(); return; }
    if (L::HATCH.contains(x, y)) { m_.clickHatch(); return; }
    if (Rect{L::DRAWER.x, L::DRAWER.y + m_.drawer.v, L::DRAWER.w, L::DRAWER.h}.contains(x, y)) {
        m_.clickDrawer();
        return;
    }
}

CursorKind App::pickCursor() const {
    if (drag_.draggingCassette()) return CursorKind::Hand;
    if (drag_.draggingMoney()) return CursorKind::Coin;
    const float x = in_.mx, y = in_.my;
    if (m_.change && L::WINDOW.contains(x, y)) return CursorKind::Key;
    if (L::HATCH.contains(x, y) && m_.mode == Mode::Idle)
        return m_.hatchLocked ? CursorKind::Padlock : CursorKind::Key;
    if (L::DOOR.contains(x, y)) {
        if (m_.mode != Mode::Cit) return CursorKind::Padlock;
        return (m_.citStep == 2 || m_.citStep == 5) ? CursorKind::Key : CursorKind::Banknote;
    }
    if (Rect{L::DRAWER.x, L::DRAWER.y + m_.drawer.v, L::DRAWER.w, L::DRAWER.h}.contains(x, y))
        return m_.drawerOpen ? CursorKind::Key : CursorKind::Padlock;
    return m_.accepting() ? CursorKind::GreenBanknote : CursorKind::Banknote;
}

// ---------------------------------------------------------------- crtanje
void App::draw() {
    r_.beginFrame(BG_COLOR);
    drawModePill();
    drawSettings();
    if (m_.mode == Mode::Cit) drawCitZones();

    MachineViewOpts o;
    o.time = time_;
    o.labels = labels_;
    o.hlCoin = drag_.hlCoin;
    o.hlBill = drag_.hlBill;
    // kaseta u mašini se crta unutar mašine (iza vrata); tek dok se vuče ide na vrh
    o.cassetteInside = m_.cas == Cas::In && drag_.kind != DragSystem::Kind::Cassette;
    o.cassettePos = drag_.casPos;
    drawMachine(r_, text_, m_, o);

    drawCassettes();
    drawCashier();
    drawTray();

    for (const auto& f : drag_.flying) drawMoney(f.bill, f.v, f.pos);
    if (drag_.draggingMoney()) drawMoney(drag_.dragBill, drag_.dragValue, drag_.dragPos);

    drawNameOverlay();
    drawCursor();          // uvek poslednji sloj
}

void App::drawModePill() {
    r_.drawRound(L::MODE_PILL, hx(0x2a2f38), 6.f);
    unsigned dot = !m_.err.empty() ? 0xef4444
        : m_.mode == Mode::Idle ? 0x6b7280 : m_.mode == Mode::Cit ? 0xef4444 : 0x22c55e;
    r_.drawEllipse({33.f, 25.f, 10.f, 10.f}, hx(dot));
    const char* name = m_.mode == Mode::Idle ? "IDLE" : m_.mode == Mode::FillUp ? "FILL UP"
                     : m_.mode == Mode::Payment ? "PAYMENT" : "CIT";
    text_.draw(std::string("Mode: ") + name, 52.f, 35.f, 13.f, hx(0xe5e7eb));
}

void App::drawSettings() {
    r_.drawRound(L::SETTINGS, hx(0x2a2f38), 6.f);
    text_.draw("Part labels", 32.f, 71.f, 12.f, hx(0xe5e7eb));
    r_.drawRound({146.f, 59.f, 32.f, 16.f}, labels_ ? hx(0x3b82c4) : hx(0x4b5563), 8.f);
    float cx = labels_ ? 170.f : 154.f;
    r_.drawEllipse({cx - 6.f, 61.f, 12.f, 12.f}, hx(0xe5e7eb));
}

void App::drawCitZones() {
    const auto col = hx(0x6b7280), txt = hx(0x374151);
    r_.drawDashed(L::BAG_ZONE, col, 6.f, 4.f);
    text_.draw("CIT bag", 755.f, 118.f, 12.f, txt, Text::Align::Center);
    if (m_.cas == Cas::InBag)
        text_.draw("Cassette: " + fmtEuroGrouped(m_.bagTotal) + " EUR", 755.f, 200.f, 11.f, txt, Text::Align::Center);
    if (m_.citStep == 4) {
        r_.drawDashed(L::NEW_ZONE, col, 6.f, 4.f);
        text_.draw("New cassette", 880.f, 118.f, 12.f, txt, Text::Align::Center);
    }
}

void App::drawCassettes() {
    // preko svega: kaseta koja se vuče, kaseta u CIT torbi i nova kaseta
    bool dragging = drag_.kind == DragSystem::Kind::Cassette;
    bool inBag = m_.cas == Cas::InBag && m_.mode == Mode::Cit;
    if (dragging || inBag) drawCassetteShape(r_, drag_.casPos, 0x5a5e66);
    if (m_.citStep == 4) drawCassetteShape(r_, drag_.ncasPos, 0x7a7f88);
}

void App::drawCashier() {
    r_.drawOutlined(L::CASHIER_PANEL, hx(0x2a2f38), hx(0x3d4450), 10.f);
    text_.draw("CASHIER", 34.f, 272.f, 12.f, hx(0x9ca3af));

    r_.drawOutlined(L::DISPLAY, hx(0x0e1a24), hx(0x3b4350), 4.f);
    DisplayLines d = cashier_.display(m_);
    const auto c = hx(0x9ec5e8), ce = hx(0xf09595);
    text_.draw(d.l1, 44.f, 300.f, 11.f, c);
    text_.draw(d.l2, 44.f, 317.f, 11.f, c);
    text_.draw(d.l3, 44.f, 334.f, 11.f, d.error ? ce : c);

    for (const Button& b : cashier_.buttons(m_)) {
        float a = (b.enabled || b.current) ? 1.f : 0.4f;
        r_.drawRound(b.rect, b.current ? hx(0x2b4a6b) : hx(0x3a414d), 5.f, a);
        text_.draw(b.label, b.rect.x + 12.f, b.rect.y + 18.f, 12.f, hx(0xe5e7eb), Text::Align::Left, a);
    }
}

void App::drawMoney(bool bill, int v, Vec2 c, float alpha) {
    if (bill) {
        r_.drawRound({c.x - L::BILL_W / 2, c.y - L::BILL_H / 2, L::BILL_W, L::BILL_H}, billColor(v), 3.f, alpha);
        text_.draw(billLabel(v), c.x, c.y + 4.f, 12.f, billText(v), Text::Align::Center, alpha);
    } else {
        const float R = L::COIN_R;
        r_.drawEllipse({c.x - R, c.y - R, 2 * R, 2 * R}, v >= 100 ? hx(0xc9ccd1) : hx(0xb9833f), alpha);
        if (v >= 100) r_.drawEllipse({c.x - 13.f, c.y - 13.f, 26.f, 26.f}, hx(0xb9833f), alpha);
        text_.draw(coinLabel(v), c.x, c.y + 4.f, 11.f, hx(0x2b1a05), Text::Align::Center, alpha);
    }
}

void App::drawTray() {
    r_.drawOutlined(L::TRAY_PANEL, hx(0x2a2f38), hx(0x3d4450), 10.f);
    text_.draw("MONEY (drag into a slot)", 684.f, 272.f, 12.f, hx(0x9ca3af));
    for (int i = 0; i < (int)BILLS.size(); ++i) drawMoney(true, BILLS[i], L::billHome(i));
    for (int i = 0; i < (int)COINS.size(); ++i) drawMoney(false, COINS[i], L::coinHome(i));
}

void App::drawNameOverlay() {
    r_.drawTex(overlayTex_, L::NAME_OVERLAY);   // poluprovidnost je u samoj teksturi (alfa kanal)
}

void App::drawCursor() {
    // hotspot = sredina slike (tu se računa klik); sistemski kursor je sakriven u main.cpp
    bool note = cur_ == CursorKind::Banknote || cur_ == CursorKind::GreenBanknote;
    const float w = note ? 48.f : 28.f, h = 28.f;
    r_.drawTex(cursorTex_[(int)cur_], {in_.mx - w * 0.5f, in_.my - h * 0.5f, w, h});
}

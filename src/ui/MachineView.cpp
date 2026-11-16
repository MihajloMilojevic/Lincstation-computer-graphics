#include "MachineView.h"
#include <cmath>
#include "gfx/Color.h"
#include "model/Money.h"
#include "ui/Layout.h"

namespace {
constexpr float PI = 3.14159265f;

// Lijevak/poluuniformni trapez kao n horizontalnih traka (bez dodatnih tekstura)
void funnel(Renderer& r, float cx, float topY, float botY, float topHalf, float botHalf,
            const glm::vec4& col, int n = 8) {
    float h = (botY - topY) / n;
    for (int i = 0; i < n; ++i) {
        float k = (i + 0.5f) / n;
        float half = topHalf + (botHalf - topHalf) * k;
        r.drawRect({cx - half, topY + i * h, half * 2.f, h + 0.5f}, col);
    }
}
}

void drawCassetteShape(Renderer& r, Vec2 p, unsigned body) {
    r.drawOutlined({p.x, p.y, L::CAS_W, L::CAS_H}, hx(body), hx(0x2c2f35), 4.f);
    r.drawRect({p.x + L::CAS_W * 0.5f - 4.f, p.y + 4.f, 8.f, L::CAS_H - 8.f}, hx(0xf2c200));          // žuta traka-ručka
}

void drawMachine(Renderer& r, Text& t, const Machine& m, const MachineViewOpts& o) {
    const float OX = L::OX, OY = L::OY;
    auto R = [&](float x, float y, float w, float h) { return Rect{OX + x, OY + y, w, h}; };

    const auto STROKE = hx(0x3a3d43), DARK = hx(0x1a1b1e), SEAM = hx(0x44474e), BODY = hx(0x2a2c31);
    const Light lt = m.light();
    const glm::vec4 lc = lt == Light::Red ? hx(0xef4444) : lt == Light::Green ? hx(0x22c55e) : hx(0x3b82f6);
    // spori, dugi puls (~3 s po ciklusu) preko uAlpha uniforma
    const float pulse = 0.25f + 0.75f * (0.5f + 0.5f * std::sin(2.f * PI * o.time / 3.f));

    // --- senka, telo ---
    r.drawRound(R(44, 452, 310, 6), hx(0x000000, 0.12f), 3.f);
    r.drawOutlined(R(44, 0, 310, 450), BODY, DARK, 6.f, 1.5f);

    // --- unutrašnjost: gore (vidi se kad je poklopac podignut) ---
    r.drawRect(R(44, 0, 310, 210), hx(0x17181b));
    r.drawRect(R(44, 0, 8, 210), hx(0x3d7a2e));                    // zeleni bočni nosači
    r.drawRect(R(346, 0, 8, 210), hx(0x3d7a2e));
    r.drawRect(R(64, 72, 54, 20), hx(0xcfd3d9));                   // specifikaciona pločica
    r.drawOutlined(R(170, 66, 60, 34), hx(0x2b2d31), STROKE, 3.f); // crveni coin skener
    r.drawRect(R(176, 74, 48, 16), hx(0xe03030));
    funnel(r, OX + 200, OY + 106, OY + 140, 34, 16, hx(0x0c0d0f));

    // --- unutrašnjost: sredina (vidi se kad je vrata otvorena) ---
    r.drawRect(R(44, 216, 310, 169), hx(0x17181b));
    r.drawOutlined(R(64, 238, 126, 130), hx(0x3a3d43), DARK, 4.f);  // SMART coin unit
    for (int i = 0; i < 6; ++i)                                      // kosi gornji deo jedinice
        r.drawRect(R(80, 238 + i * 4.f, 70 + 30 * (i + 0.5f) / 6.f, 4.5f), hx(0x26282c));
    r.drawRound(R(226, 222, 92, 14), hx(0x1a1b1e), 3.f);            // MEI SCR Advance recycler
    r.drawOutlined(R(232, 236, 80, 66), hx(0xf2c200), hx(0x8a6d00), 6.f);
    r.drawRect(R(240, 252, 64, 22), hx(0xffffff));
    if (o.cassetteInside) drawCassetteShape(r, o.cassettePos, 0x5a5e66);   // iza vrata: vrata je prekrivaju dok su zatvorena

    // procep iznad izvučene fioke
    if (m.drawer.v > 0.f) r.drawRect(R(56, 385, 286, m.drawer.v + 3), hx(0x0f1012));
    r.drawRect(R(44, 385, 310, 3), SEAM);

    // --- gornji poklopac (savija se od vrha) ---
    r.setXform(OY, m.hatch.v, 0.f);
    r.drawOutlined(R(44, 0, 310, 213), BODY, DARK, 6.f, 1.5f);
    r.drawRect(R(44, 210, 310, 3), SEAM);

    // touchscreen
    r.drawOutlined(R(64, 38, 104, 64), hx(0x0d0e10), STROKE, 4.f);
    r.drawRound(R(72, 45, 88, 50), hx(0x1c2b3b), 2.f);
    t.draw(m.touch1(), OX + 116, OY + 66, 12, hx(0x9ec5e8), Text::Align::Center);
    t.draw(m.touch2(), OX + 116, OY + 84, 11, hx(0x6b8ba8), Text::Align::Center);

    // coin slot
    r.drawOutlined(R(172, 30, 60, 70), hx(0x14161a), o.hlCoin ? hx(0xffffff) : STROKE, 6.f);
    r.drawRound(R(178, 38, 48, 5), lc, 2.f, pulse);
    funnel(r, OX + 202, OY + 50, OY + 90, 22, 18, hx(0x0a0b0d), 6);
    for (float y : {62.f, 72.f, 82.f}) r.drawRect(R(183, y - 1, 38, 2), hx(0x1f2a36));

    // bill slot
    r.drawOutlined(R(258, 34, 84, 66), hx(0x14161a), o.hlBill ? hx(0xffffff) : STROKE, 8.f);
    r.drawOutlined(R(268, 48, 64, 12), hx(0x050607), hx(0x2a2d33), 6.f);
    r.drawEllipseOutlined(R(278, 67, 44, 14), hx(0x23262b), STROKE);
    r.drawRound(R(264, 89, 72, 5), lc, 2.f, pulse);

    // statusne LED diode
    r.drawOutlined(R(188, 112, 26, 12), hx(0x0d0e10), STROKE, 2.f);
    r.drawEllipse(R(193.5f, 115.5f, 5, 5), lc, pulse);
    r.drawEllipse(R(203.5f, 115.5f, 5, 5), lc, pulse);

    // --- srednja vrata (savijaju se od dna) ---
    r.setXform(OY + 385, m.door.v, 0.f);
    r.drawOutlined(R(44, 216, 310, 169), hx(0x2d2f35), DARK, 0.f);
    r.drawRect(R(44, 216, 310, 3), SEAM);
    r.drawOutlined(R(162, 252, 76, 36), hx(0xc9ced6), hx(0x8b919a), 2.f);   // otvor za kusur
    r.drawRect(R(170, 259, 60, 22), hx(0x1a1c20));
    if (m.change) {
        r.drawRound(R(146, 291, 108, 18), hx(0x3b82f6), 9.f);
        t.draw("Take " + fmtEur(*m.change) + " EUR", OX + 200, OY + 304, 11, hx(0xffffff), Text::Align::Center);
    }

    // --- fioka (izvlači se naniže) ---
    r.setXform(0.f, 1.f, m.drawer.v);
    r.drawOutlined(R(44, 388, 310, 62), hx(0x2d2f35), DARK, 0.f);
    r.drawRound(R(160, 402, 80, 8), hx(0x1a1b1e), 3.f);
    r.resetXform();

    // --- opcione labele delova ---
    if (o.labels) {
        struct Lb { const char* s; float x, y, w; };
        static const Lb lb[] = {
            {"Top hatch", 164, 141, 70}, {"Touchscreen", 75, 70, 82}, {"Coin slot", 169, 62, 66},
            {"Bill slot", 265, 52, 70}, {"Middle door", 60, 226, 82}, {"Dispense window", 146, 315, 108},
            {"Coin drawer", 158, 409, 82}};
        for (const Lb& l : lb) {
            r.drawRound(R(l.x, l.y, l.w, 18), hx(0xffffff), 9.f, 0.92f);
            t.draw(l.s, OX + l.x + l.w / 2, OY + l.y + 13, 11, hx(0x1b1e24), Text::Align::Center);
        }
    }
}

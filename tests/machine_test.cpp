// Headless testovi logike (bez OpenGL-a): Machine + Cashier.
#include <cassert>
#include <cstdio>
#include "model/Cashier.h"
#include "model/Machine.h"
#include "model/Money.h"

static int checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

int main() {
    // --- svetla i zaključavanje ---
    {
        Machine m;
        CHECK(m.light() == Light::Blue);
        CHECK(m.drop(true, Slot::Bill) == Drop::Reject);          // IDLE: slotovi ne primaju
        CHECK(m.msg == "Slots are not accepting");
        m.toggleHatchLock();                                      // otključan poklopac -> crveno
        CHECK(m.light() == Light::Red);
        CHECK(!m.canFill() && !m.canPay() && !m.canCit());
        m.toggleHatchLock();
        CHECK(m.light() == Light::Blue);
        m.clickHatch();                                           // zaključan
        CHECK(m.msg == "Top hatch is locked" && !m.hatchOpen);
        m.clickDoor();
        CHECK(m.msg == "Middle door is locked");
        m.clickDrawer();
        CHECK(m.msg == "Coin drawer is locked");
        m.toggleDrawer();                                         // fioka se otključava i izvlači
        CHECK(m.drawerOpen && m.light() == Light::Red);
        m.clickDrawer();                                          // uvlači se i ponovo je zaključana
        CHECK(!m.drawerOpen);
        m.drawer.v = 0.f;
        CHECK(m.light() == Light::Blue);
    }
    // --- FILL UP ---
    {
        Machine m;
        m.pressFill();
        CHECK(m.mode == Mode::FillUp && m.accepting() && m.light() == Light::Green);
        CHECK(m.drop(true, Slot::Coin) == Drop::Reject && m.light() == Light::Red);   // pogrešan slot
        CHECK(m.drop(true, Slot::Bill) == Drop::Accept);
        int before = m.counts[2000];
        m.addMoney(2000);
        CHECK(m.filled == 2000 && m.counts[2000] == before + 1);
        m.pressFill();                                            // Done
        CHECK(m.mode == Mode::Idle && m.msg == "Stored 20.00 EUR");
    }
    // --- PAYMENT: tačan iznos, kusur, nema tačnog kusura ---
    {
        Machine m;
        m.due = 700;
        m.pressPayment();
        m.addMoney(500);
        CHECK(m.mode == Mode::Payment && m.paid == 500);
        m.addMoney(200);
        CHECK(m.mode == Mode::Idle && !m.change && m.msg == "Paid exactly");
    }
    {
        Machine m;
        m.due = 300;
        m.pressPayment();
        m.addMoney(500);                                          // kusur 2.00
        CHECK(m.mode == Mode::Idle && m.change && *m.change == 200);
        CHECK(m.touch1() == "Take change" && !m.canPay());        // blokirano dok se kusur ne preuzme
        CHECK(m.takeChange() && !m.change);
        CHECK(m.canPay());
    }
    {
        Machine m;
        m.counts.clear();                                         // prazna mašina
        m.due = 10;
        m.pressPayment();
        m.addMoney(500);                                          // kusur 4.90 nemoguć
        CHECK(!m.err.empty() && m.err == "No exact change. Money returned");
        CHECK(m.counts[500] == 0 && !m.change && m.light() == Light::Red);
        m.update(4.5f);                                           // greška ističe za ~4 s
        CHECK(m.err.empty() && m.light() == Light::Blue);
    }
    // --- CIT ---
    {
        Machine m;
        m.pressCit();
        CHECK(m.mode == Mode::Cit && m.citStep == 2 && m.light() == Light::Red);
        m.clickDoor();
        CHECK(m.doorOpen && m.citStep == 3);
        CHECK(!m.canGrabCassette());                              // vrata se još otvaraju
        m.update(1.0f);
        CHECK(m.canGrabCassette());
        m.cassetteToBag();
        CHECK(m.bagTotal == 37000 && m.citStep == 4 && m.cas == Cas::InBag);
        for (int b : BILLS) CHECK(m.counts[b] == 0);
        for (int c : COINS) CHECK(m.counts[c] == 20);             // kovanice nisu dirnute
        CHECK(fmtEuroGrouped(m.bagTotal) == "370");
        CHECK(fmtEuroGrouped(124000) == "1,240");
        m.newCassetteIn();
        CHECK(m.citStep == 5);
        m.clickDoor();
        CHECK(m.mode == Mode::Idle && !m.doorOpen && m.cas == Cas::In && m.msg == "CIT complete");
    }
    // --- kucanje iznosa ---
    {
        Machine m;
        Cashier c;
        Input in;
        in.typed = "12.5";  c.handleTyping(in, m);  CHECK(c.buf == "12.5");
        in.typed = "99";    c.handleTyping(in, m);  CHECK(c.buf == "12.59");  // max 2 decimale (drugo 9 odbijeno)
        in.typed.clear(); in.enter = true;          c.handleTyping(in, m);
        CHECK(m.due == 1259 && c.buf.empty());
        in = Input(); in.typed = "0"; c.handleTyping(in, m);
        in = Input(); in.enter = true; c.handleTyping(in, m);
        CHECK(m.due == 1259 && m.msg == "Invalid amount");            // 0 se odbija
        in = Input(); in.typed = "7.05"; c.handleTyping(in, m);
        in = Input(); in.backspace = true; c.handleTyping(in, m);
        CHECK(c.buf == "7.0");
        in = Input(); in.enter = true; c.handleTyping(in, m);
        CHECK(m.due == 700);
        CHECK(Cashier::parseCents(".5") == 50 && Cashier::parseCents("3") == 300 && Cashier::parseCents("0.07") == 7);
        m.pressPayment();                                             // u PAYMENT modu kucanje se ignoriše
        in = Input(); in.typed = "5"; c.handleTyping(in, m);
        CHECK(c.buf.empty());
    }
    // --- dugmad: CIT lista koraka nije klikabilna ---
    {
        Machine m;
        Cashier c;
        auto b = c.buttons(m);
        CHECK(b.size() == 5 && b[0].enabled && b[4].enabled && b[0].label == "Fill up");
        m.pressCit();
        b = c.buttons(m);
        for (auto& x : b) CHECK(!x.enabled);
        CHECK(b[1].current && b[0].label.find("Start CIT") != std::string::npos);
    }
    std::printf("OK: %d provera\n", checks);
    return 0;
}

#include "Cashier.h"
#include <cctype>
#include "model/Money.h"
#include "ui/Layout.h"

int Cashier::parseCents(const std::string& s) {
    size_t dot = s.find('.');
    std::string whole = s.substr(0, dot);
    std::string frac = dot == std::string::npos ? "" : s.substr(dot + 1);
    while (frac.size() < 2) frac.push_back('0');
    int w = whole.empty() ? 0 : std::stoi(whole);
    return w * 100 + std::stoi(frac.substr(0, 2));
}

void Cashier::handleTyping(const Input& in, Machine& m) {
    if (m.mode != Mode::Idle) { buf.clear(); return; }

    for (char c : in.typed) {
        size_t dot = buf.find('.');
        if (c == '.') {
            if (dot == std::string::npos && buf.size() < 7) buf.push_back('.');
        } else if (std::isdigit((unsigned char)c)) {
            if (dot != std::string::npos && buf.size() - dot - 1 >= 2) continue;   // najviše 2 decimale
            if (buf.size() < 7) buf.push_back(c);
        }
    }
    if (in.backspace && !buf.empty()) buf.pop_back();

    if (in.enter && !buf.empty()) {
        int cents = parseCents(buf);
        if (cents >= 1 && cents <= 99999) m.due = cents;             // do 999.99 EUR
        else m.setMsg("Invalid amount");
        buf.clear();
    }
}

std::vector<Button> Cashier::buttons(Machine& m) const {
    std::vector<Button> b;
    auto rect = [](int i) { return Rect{L::BTN_X, L::BTN_Y[i], L::BTN_W, L::BTN_H}; };

    if (m.mode == Mode::Cit) {
        static const char* names[5] = {"Start CIT", "Open middle door", "Remove cassette",
                                       "Insert new cassette", "Close middle door"};
        static const char* hints[6] = {"", "", "Click the middle door to open", "Drag the cassette to the bag",
                                       "Drag the new cassette in", "Click the middle door to close"};
        for (int i = 0; i < 5; ++i) {
            int n = i + 1;
            bool cur = n > 1 && m.citStep == n;
            bool done = n == 1 || m.citStep > n;
            std::string label = done ? "\xE2\x9C\x93 " : std::to_string(n) + "  ";   // ✓
            label += cur ? hints[n] : names[i];
            b.push_back({label, rect(i), false, cur, nullptr});
        }
        return b;
    }

    b.push_back({m.mode == Mode::FillUp ? "Done" : "Fill up", rect(0), m.canFill(), false,
                 [&m] { m.pressFill(); }});
    b.push_back({"Payment", rect(1), m.canPay(), false, [&m] { m.pressPayment(); }});
    b.push_back({m.hatchLocked ? "Unlock top hatch" : "Lock top hatch", rect(2), m.canToggleHatch(), false,
                 [&m] { m.toggleHatchLock(); }});
    b.push_back({m.drawerOpen ? "Close coin drawer" : "Open coin drawer", rect(3), m.canDrawer(), false,
                 [&m] { m.toggleDrawer(); }});
    b.push_back({"CIT procedure", rect(4), m.canCit(), false, [&m] { m.pressCit(); }});
    return b;
}

DisplayLines Cashier::display(const Machine& m) const {
    DisplayLines d;
    std::string name = m.mode == Mode::Idle ? "IDLE" : m.mode == Mode::FillUp ? "FILL UP"
                     : m.mode == Mode::Payment ? "PAYMENT" : "CIT";
    d.l1 = "State: " + name;

    switch (m.mode) {
        case Mode::Idle:
            d.l2 = buf.empty() ? "Due: " + fmtEur(m.due) + " EUR" : "Due: " + buf + "_  (Enter to set)";
            d.l3 = !m.msg.empty() ? m.msg
                 : m.change ? "Take change: " + fmtEur(*m.change) + " EUR" : "Change: \xE2\x80\x94";   // —
            break;
        case Mode::FillUp:
            d.l2 = "Stored: " + fmtEur(m.filled) + " EUR";
            d.l3 = !m.msg.empty() ? m.msg : "Drag money into the slots";
            break;
        case Mode::Payment:
            d.l2 = "Due " + fmtEur(m.due) + " / Paid " + fmtEur(m.paid);
            d.l3 = !m.msg.empty() ? m.msg : "Remaining: " + fmtEur(m.due > m.paid ? m.due - m.paid : 0);
            break;
        case Mode::Cit:
            d.l2 = "Step " + std::to_string(m.citStep) + " of 5";
            d.l3 = m.msg;
            break;
    }
    if (!m.err.empty()) { d.l1 = "State: ERROR"; d.l3 = m.err; d.error = true; }
    return d;
}

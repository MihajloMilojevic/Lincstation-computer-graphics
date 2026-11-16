#include "Machine.h"
#include "Money.h"

Machine::Machine() {
    const int nb[] = {6, 6, 4, 2, 1};                       // početna zaliha: 6x5, 6x10, 4x20, 2x50, 1x100
    for (size_t i = 0; i < BILLS.size(); ++i) counts[BILLS[i]] = nb[i];
    for (int c : COINS) counts[c] = 20;
}

bool Machine::accepting() const {
    return (mode == Mode::FillUp || mode == Mode::Payment) && !hatchOpen && hatch.v > 0.98f;
}

bool Machine::allClear() const {
    return hatchLocked && !hatchOpen && !doorOpen && !drawerOpen && err.empty() && !change;
}

Light Machine::light() const {
    if (!err.empty() || flashTimer > 0.f || mode == Mode::Cit || !hatchLocked || hatchOpen ||
        doorOpen || drawerOpen || drawer.v > 0.f)
        return Light::Red;
    if (mode == Mode::FillUp || mode == Mode::Payment) return Light::Green;
    return Light::Blue;
}

std::string Machine::touch1() const {
    if (!err.empty()) return "Error";
    switch (mode) {
        case Mode::FillUp:  return "Insert money";
        case Mode::Payment: return "Please pay";
        case Mode::Cit:     return "CIT mode";
        default: break;
    }
    return change ? "Take change" : "Ready";
}

std::string Machine::touch2() const {
    if (!err.empty()) return "";
    switch (mode) {
        case Mode::FillUp:  return fmtEur(filled) + " EUR";
        case Mode::Payment: return fmtEur(due > paid ? due - paid : 0) + " EUR";
        case Mode::Cit:     return "";
        default: break;
    }
    return change ? fmtEur(*change) + " EUR" : "";
}

void Machine::setMsg(const std::string& t) { msg = t; msgTimer = 3.f; }
void Machine::setError(const std::string& t) { err = t; errTimer = 4.f; }

// ---------------- dugmad ----------------
bool Machine::canFill() const { return mode == Mode::FillUp || (mode == Mode::Idle && allClear()); }
void Machine::pressFill() {
    if (mode == Mode::FillUp) {
        mode = Mode::Idle;
        setMsg("Stored " + fmtEur(filled) + " EUR");
    } else if (canFill()) {
        mode = Mode::FillUp;
        filled = 0;
    }
}

bool Machine::canPay() const { return mode == Mode::Idle && allClear() && due > 0; }
void Machine::pressPayment() {
    if (!canPay()) return;
    mode = Mode::Payment;
    paid = 0;
    inserted.clear();
}

bool Machine::canToggleHatch() const { return mode == Mode::Idle && !hatchOpen; }
void Machine::toggleHatchLock() { if (canToggleHatch()) hatchLocked = !hatchLocked; }

bool Machine::canDrawer() const { return mode == Mode::Idle; }
void Machine::toggleDrawer() { if (canDrawer()) drawerOpen = !drawerOpen; }

bool Machine::canCit() const { return mode == Mode::Idle && allClear(); }
void Machine::pressCit() {
    if (!canCit()) return;
    mode = Mode::Cit;
    citStep = 2;
}

// ---------------- klikovi na mašinu ----------------
void Machine::clickHatch() {
    if (mode != Mode::Idle) return;
    if (hatchLocked) setMsg("Top hatch is locked");
    else hatchOpen = !hatchOpen;
}

void Machine::clickDoor() {
    if (mode == Mode::Cit) {
        if (citStep == 2) {                       // korak 2: otvori vrata
            doorOpen = true;
            citStep = 3;
        } else if (citStep == 5) {                // korak 5: zatvori vrata, kraj procedure
            doorOpen = false;
            mode = Mode::Idle;
            citStep = 0;
            cas = Cas::In;
            setMsg("CIT complete");
        }
    } else {
        setMsg("Middle door is locked");
    }
}

void Machine::clickDrawer() {
    if (drawerOpen) drawerOpen = false;           // kad se uvuče, ponovo je zaključana
    else setMsg("Coin drawer is locked");
}

bool Machine::takeChange() {
    if (!change) return false;
    int t = *change;
    change.reset();
    setMsg("Taken " + fmtEur(t) + " EUR");
    return true;
}

// ---------------- novac ----------------
Drop Machine::drop(bool isBill, Slot slot) {
    if (slot == Slot::None) return Drop::Reject;
    bool ok = accepting() && ((slot == Slot::Coin && !isBill) || (slot == Slot::Bill && isBill));
    if (ok) return Drop::Accept;
    if (accepting()) { flashTimer = 0.9f; setMsg("Wrong slot"); }
    else setMsg("Slots are not accepting");
    return Drop::Reject;
}

void Machine::addMoney(int cents) {
    counts[cents]++;
    if (mode == Mode::FillUp) {
        filled += cents;
    } else if (mode == Mode::Payment) {
        paid += cents;
        inserted.push_back(cents);
        if (paid >= due) settle();
    }
}

void Machine::settle() {
    int rem = paid - due;
    std::vector<std::pair<int, int>> take;
    for (auto it = counts.rbegin(); it != counts.rend(); ++it) {        // od najveće denominacije
        int n = std::min(it->second, rem / it->first);
        if (n > 0) { rem -= n * it->first; take.push_back({it->first, n}); }
    }
    mode = Mode::Idle;
    if (rem > 0) {                                                       // nema tačnog kusura
        for (int v : inserted) counts[v]--;
        setError("No exact change. Money returned");
    } else {
        for (auto& [d, n] : take) counts[d] -= n;
        int ch = paid - due;
        if (ch > 0) change = ch;
        setMsg(ch > 0 ? "Payment stored" : "Paid exactly");
    }
    paid = 0;
    inserted.clear();
}

// ---------------- CIT ----------------
bool Machine::canGrabCassette() const {
    return mode == Mode::Cit && citStep == 3 && cas == Cas::In && door.v < 0.5f;
}
bool Machine::canGrabNewCassette() const { return mode == Mode::Cit && citStep == 4; }

void Machine::cassetteToBag() {
    int tot = 0;
    for (int b : BILLS) { tot += b * counts[b]; counts[b] = 0; }       // kovanice ostaju u coin unit-u
    bagTotal = tot;
    cas = Cas::InBag;
    citStep = 4;
    setMsg("Cassette removed");
}

void Machine::newCassetteIn() {
    cas = Cas::In;
    citStep = 5;
    setMsg("New cassette inserted");
}

// ---------------- vreme ----------------
void Machine::update(float dt) {
    hatch.target  = hatchOpen ? 0.2f : 1.f;
    door.target   = doorOpen ? 0.18f : 1.f;
    drawer.target = drawerOpen ? 28.f : 0.f;
    hatch.update(dt);
    door.update(dt);
    drawer.update(dt);

    if (msgTimer > 0.f && (msgTimer -= dt) <= 0.f) msg.clear();
    if (errTimer > 0.f && (errTimer -= dt) <= 0.f) err.clear();
    if (flashTimer > 0.f) flashTimer -= dt;
}

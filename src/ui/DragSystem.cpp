#include "DragSystem.h"
#include <cmath>
#include "ui/Layout.h"

Vec2 DragSystem::L_CAS_HOME()  { return L::CAS_HOME; }
Vec2 DragSystem::L_NCAS_HOME() { return L::NCAS_HOME; }

Slot DragSystem::slotAt(float x, float y) {
    if (L::COIN_SLOT.contains(x, y)) return Slot::Coin;
    if (L::BILL_SLOT.contains(x, y)) return Slot::Bill;
    return Slot::None;
}

void DragSystem::startMoney(bool bill, int v, Vec2 home, const Input& in) {
    if (active()) return;
    kind = Kind::Money;
    dragBill = bill;
    dragValue = v;
    home_ = home;
    dragPos = {in.mx, in.my};
}

void DragSystem::startCassette(Kind k, const Input& in) {
    if (active()) return;
    kind = k;
    Vec2 p = k == Kind::Cassette ? casPos : ncasPos;
    grab_ = {in.mx - p.x, in.my - p.y};
}

static bool step(Vec2& p, Vec2 target, float speed, float dt) {     // konstantna brzina; true = stigao
    float dx = target.x - p.x, dy = target.y - p.y;
    float d = std::sqrt(dx * dx + dy * dy), s = speed * dt;
    if (d <= s) { p = target; return true; }
    p.x += dx / d * s;
    p.y += dy / d * s;
    return false;
}

void DragSystem::update(float dt, const Input& in, Machine& m) {
    // novac u letu (u slot ili nazad u traku)
    for (size_t i = 0; i < flying.size();) {
        if (step(flying[i].pos, flying[i].target, flying[i].speed, dt)) {
            if (flying[i].deliver) m.addMoney(flying[i].v);
            flying.erase(flying.begin() + i);
        } else {
            ++i;
        }
    }

    hlCoin = hlBill = false;
    if (kind == Kind::Money) {
        dragPos = {in.mx, in.my};
        Slot s = slotAt(in.mx, in.my);
        bool ok = m.accepting() && ((s == Slot::Coin && !dragBill) || (s == Slot::Bill && dragBill));
        hlCoin = ok && s == Slot::Coin;
        hlBill = ok && s == Slot::Bill;
    } else if (kind == Kind::Cassette) {
        casPos = {in.mx - grab_.x, in.my - grab_.y};
    } else if (kind == Kind::NewCassette) {
        ncasPos = {in.mx - grab_.x, in.my - grab_.y};
    }

    if (in.lmbReleased && active()) release(in, m);
}

void DragSystem::release(const Input& in, Machine& m) {
    Vec2 mouse{in.mx, in.my};
    Kind k = kind;
    kind = Kind::None;
    hlCoin = hlBill = false;

    if (k == Kind::Money) {
        Slot s = slotAt(mouse.x, mouse.y);
        if (m.drop(dragBill, s) == Drop::Accept)
            flying.push_back({dragBill, dragValue, mouse, s == Slot::Coin ? L::COIN_TARGET : L::BILL_TARGET, 500.f, true});
        else
            flying.push_back({dragBill, dragValue, mouse, home_, 900.f, false});
    } else if (k == Kind::Cassette) {
        Vec2 c{casPos.x + L::CAS_W / 2, casPos.y + L::CAS_H / 2};
        if (L::BAG_ZONE.contains(c.x, c.y)) { m.cassetteToBag(); casPos = L::CAS_IN_BAG; }
        else casPos = L::CAS_HOME;
    } else if (k == Kind::NewCassette) {
        Vec2 c{ncasPos.x + L::CAS_W / 2, ncasPos.y + L::CAS_H / 2};
        if (L::INSERT_ZONE.contains(c.x, c.y)) { m.newCassetteIn(); casPos = L::CAS_HOME; }
        ncasPos = L::NCAS_HOME;
    }
}

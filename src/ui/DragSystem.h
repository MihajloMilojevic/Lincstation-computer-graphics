#pragma once
#include <vector>
#include "core/Input.h"
#include "core/Rect.h"
#include "model/Machine.h"

// Prevlačenje novca iz trake u slotove i kasete u CIT zone + animacije povratka/uvlačenja
// (sve konstantnom brzinom: povratak 900 px/s, uvlačenje u slot 500 px/s).
class DragSystem {
public:
    enum class Kind { None, Money, Cassette, NewCassette };

    struct Flying {                 // novac koji leti u slot ili nazad u traku
        bool bill; int v;
        Vec2 pos, target;
        float speed;
        bool deliver;               // true: po dolasku se računa u mašinu
    };

    // vidljivo App-u za crtanje
    Kind kind = Kind::None;
    bool dragBill = false;
    int dragValue = 0;
    Vec2 dragPos{0, 0};             // centar novca koji se vuče
    Vec2 casPos = L_CAS_HOME(), ncasPos = L_NCAS_HOME();
    bool hlCoin = false, hlBill = false;
    std::vector<Flying> flying;

    bool active() const { return kind != Kind::None; }
    bool draggingMoney() const { return kind == Kind::Money; }
    bool draggingCassette() const { return kind == Kind::Cassette || kind == Kind::NewCassette; }

    void startMoney(bool bill, int v, Vec2 home, const Input& in);
    void startCassette(Kind k, const Input& in);
    void update(float dt, const Input& in, Machine& m);

    static Slot slotAt(float x, float y);

private:
    static Vec2 L_CAS_HOME();
    static Vec2 L_NCAS_HOME();
    void release(const Input& in, Machine& m);

    Vec2 home_{0, 0};
    Vec2 grab_{0, 0};               // offset hvatanja kasete
};

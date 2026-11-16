#pragma once
#include <functional>
#include <string>
#include <vector>
#include "core/Input.h"
#include "core/Rect.h"
#include "model/Machine.h"

// Dugme kasirskog panela. Dugmad su tabela {label, rect, enabled, onClick};
// u CIT modu se ista tabela gradi kao lista koraka koja se ne može kliknuti.
struct Button {
    std::string label;
    Rect rect;
    bool enabled = false;
    bool current = false;               // istaknut trenutni CIT korak
    std::function<void()> onClick;
};

struct DisplayLines { std::string l1, l2, l3; bool error = false; };

class Cashier {
public:
    std::string buf;                    // iznos koji se kuca (npr. "12.5")

    // cifre, tačka (i zarez), Backspace, Enter; radi samo u IDLE stanju
    void handleTyping(const Input& in, Machine& m);

    std::vector<Button> buttons(Machine& m) const;
    DisplayLines display(const Machine& m) const;

    static int parseCents(const std::string& s);   // "12.5" -> 1250, ".5" -> 50
};

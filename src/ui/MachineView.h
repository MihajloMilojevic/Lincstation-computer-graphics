#pragma once
#include "gfx/Renderer.h"
#include "gfx/Text.h"
#include "core/Rect.h"
#include "model/Machine.h"

// Crta mašinu (front view): telo, unutrašnjost, gornji blok, srednja vrata, fioka,
// svetla, opcione labele delova. Sve iz ravnih oblika, po koordinatama iz prototipa.
struct MachineViewOpts {
    float time = 0.f;
    bool labels = false;
    bool hlCoin = false, hlBill = false;   // beli obrub slota dok validan novac lebdi iznad njega
    bool cassetteInside = false;           // kaseta je u mašini: crta se iza vrata, pa je vrata prirodno prekrivaju
    Vec2 cassettePos{0.f, 0.f};            // gornji levi ugao kasete (referentne koordinate)
};

void drawMachine(Renderer& r, Text& t, const Machine& m, const MachineViewOpts& o);

// Kaseta sa žutom trakom-ručkom; koristi je i mašina (iza vrata) i App (dok se vuče / u torbi)
void drawCassetteShape(Renderer& r, Vec2 p, unsigned body);

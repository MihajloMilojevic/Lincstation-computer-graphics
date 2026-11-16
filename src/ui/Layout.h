#pragma once
#include "core/Rect.h"

// Sve referentne koordinate (ekran 960x540) iz HTML prototipa na jednom mestu.
namespace L {

// Mašina: telo je u prototipu pomereno za (281, 40)
constexpr float OX = 281.f, OY = 40.f;
constexpr Rect MACHINE {325.f, 40.f, 310.f, 450.f};
constexpr Rect HATCH   {325.f, 40.f, 310.f, 213.f};    // ceo gornji blok
constexpr Rect DOOR    {325.f, 256.f, 310.f, 169.f};   // srednja vrata
constexpr Rect DRAWER  {325.f, 428.f, 310.f, 62.f};    // fioka za kovanice (kad je uvučena)
constexpr Rect WINDOW  {427.f, 288.f, 108.f, 62.f};    // otvor za kusur
constexpr Rect COIN_SLOT {443.f, 60.f, 80.f, 90.f};
constexpr Rect BILL_SLOT {529.f, 64.f, 104.f, 86.f};
constexpr Vec2 COIN_TARGET {483.f, 105.f};
constexpr Vec2 BILL_TARGET {581.f, 94.f};

// Kaseta i CIT zone
constexpr float CAS_W = 56.f, CAS_H = 40.f;
constexpr Vec2 CAS_HOME  {525.f, 348.f};
constexpr Vec2 CAS_IN_BAG{723.f, 140.f};
constexpr Vec2 NCAS_HOME {848.f, 135.f};
constexpr Rect BAG_ZONE    {690.f, 100.f, 130.f, 110.f};
constexpr Rect NEW_ZONE    {830.f, 100.f, 100.f, 110.f};
constexpr Rect INSERT_ZONE {500.f, 330.f, 110.f, 70.f};

// HUD
constexpr Rect MODE_PILL     {20.f, 16.f, 170.f, 28.f};
constexpr Rect SETTINGS      {20.f, 50.f, 170.f, 34.f};
constexpr Rect NAME_OVERLAY  {700.f, 16.f, 240.f, 44.f};
constexpr Rect CASHIER_PANEL {20.f, 250.f, 270.f, 270.f};
constexpr Rect DISPLAY       {34.f, 282.f, 242.f, 62.f};
constexpr float BTN_X = 34.f, BTN_W = 242.f, BTN_H = 28.f;
constexpr float BTN_Y[5] = {354.f, 386.f, 418.f, 450.f, 482.f};
constexpr Rect TRAY_PANEL    {670.f, 250.f, 270.f, 270.f};

// Ikone novca u traci
constexpr Vec2 billHome(int i) { return {739.f, 301.f + i * 40.f}; }
constexpr Vec2 coinHome(int i) { return {880.f, 304.f + i * 46.f}; }
constexpr float BILL_W = 66.f, BILL_H = 32.f, COIN_R = 20.f;

}  // namespace L

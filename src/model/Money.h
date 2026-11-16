#pragma once
#include <array>
#include <cstdio>
#include <string>

// Novac se čuva u centima (int), bez floatova.
inline constexpr std::array<int, 5> BILLS {500, 1000, 2000, 5000, 10000};
inline constexpr std::array<int, 5> COINS {10, 20, 50, 100, 200};

inline std::string fmtEur(int cents) {
    char b[32];
    std::snprintf(b, sizeof b, "%d.%02d", cents / 100, cents % 100);
    return b;
}

// 124000 centi -> "1,240" (celi eviri sa separatorom hiljada, kao u prototipu)
inline std::string fmtEuroGrouped(int cents) {
    std::string s = std::to_string(cents / 100), out;
    int n = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.insert(out.begin(), s[i]);
        if (++n % 3 == 0 && i > 0) out.insert(out.begin(), ',');
    }
    return out;
}

inline std::string billLabel(int cents) { return std::to_string(cents / 100) + " \xE2\x82\xAC"; }   // "5 €"
inline std::string coinLabel(int cents) { return cents < 100 ? fmtEur(cents) : std::to_string(cents / 100); }

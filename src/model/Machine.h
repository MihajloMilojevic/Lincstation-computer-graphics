#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "core/Anim.h"

enum class Mode  { Idle, FillUp, Payment, Cit };
enum class Light { Blue, Green, Red };
enum class Cas   { In, InBag };
enum class Slot  { None, Coin, Bill };
enum class Drop  { Accept, Reject };

// Ceo model mašine: nema nikakve veze sa OpenGL-om (može se testirati bez prozora).
struct Machine {
    Mode mode = Mode::Idle;

    // delovi: 'locked' je pojam gornjeg poklopca, ostali se otključavaju tokom operacije
    bool hatchLocked = true, hatchOpen = false;
    bool doorOpen = false, drawerOpen = false;
    Anim hatch {1.f, 1.f, 1.2f};     // scaleY: 1 zatvoren, 0.2 otvoren (~0.7 s)
    Anim door  {1.f, 1.f, 1.2f};     // scaleY: 1 zatvorena, 0.18 otvorena
    Anim drawer{0.f, 0.f, 50.f};     // izvučenost u px (50 px/s)

    // novac (centi)
    std::map<int, int> counts;       // denominacija -> broj komada
    int due = 700, paid = 0, filled = 0;
    std::vector<int> inserted;
    std::optional<int> change;       // kusur koji čeka da se preuzme

    // CIT
    int citStep = 0;                 // 0 = nije aktivan, 2..5 = koraci
    Cas cas = Cas::In;
    int bagTotal = 0;

    // poruke
    std::string msg, err;
    float msgTimer = 0.f, errTimer = 0.f, flashTimer = 0.f;

    Machine();

    // --- izvedeno stanje ---
    bool accepting() const;
    bool allClear() const;           // sve zaključano/zatvoreno, nema greške ni kusura
    Light light() const;             // crveno > zeleno > plavo
    std::string touch1() const;      // gornja linija touchscreen-a
    std::string touch2() const;

    void setMsg(const std::string& t);
    void setError(const std::string& t);

    // --- dugmad kasira ---
    bool canFill() const;     void pressFill();          // Fill up / Done
    bool canPay() const;      void pressPayment();
    bool canToggleHatch() const; void toggleHatchLock();
    bool canDrawer() const;   void toggleDrawer();
    bool canCit() const;      void pressCit();

    // --- klikovi na mašinu ---
    void clickHatch();
    void clickDoor();
    void clickDrawer();
    bool takeChange();

    // --- novac ---
    Drop drop(bool isBill, Slot slot);   // procena otpuštanja (postavlja poruku/treptanje)
    void addMoney(int cents);            // novčanica/kovanica je prošla kroz slot
    void settle();                       // kusur greedy-jem ili povraćaj

    // --- CIT ---
    bool canGrabCassette() const;
    bool canGrabNewCassette() const;
    void cassetteToBag();
    void newCassetteIn();

    void update(float dt);
};

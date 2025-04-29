#ifndef SWITCH_H
#define SWITCH_H

#include "component.h"

// Kapcsoló osztály
class Switch : public component {
private:
    bool isOn;

public:
    // Konstruktor: kezdeti állapot beállítása
    Switch(bool initialState = false) : isOn(initialState) {}

    // Állapot változtatása
    void toggle();

    // Aktuális állapot lekérdezése
    bool getState() const override;
};

#endif // SWITCH_H

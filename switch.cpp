#include "switch.h"

// Kapcsoló állapotának megfordítása
void Switch::toggle() {
    isOn = !isOn;
}

// Kapcsoló aktuális állapotának lekérdezése
bool Switch::getState() const {
    return isOn;
}

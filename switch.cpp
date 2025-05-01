#include "switch.h"

// Állapot fel-/lekapcsolása
void Switch::toggle() {
    isOn = !isOn;
}

// Explicit állapotbeállítás
void Switch::setState(const bool s) {
    isOn = s;
}

bool Switch::getState() const {
    return isOn;
}

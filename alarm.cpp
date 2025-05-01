#include "alarm.h"

// Trigger beállítása
void alarm::setTrigger(component* c) {
    trigger = c;
}

// Aktív/inaktív állapot
void alarm::update() {
    if (trigger)
        active = trigger->getState();
    else
        active = false;
}

// Állapot lekérdezése
bool alarm::getState() const {
    return active;
}

// Aktuális trigger visszaadása
component* alarm::getTrigger() const {
    return trigger;
}

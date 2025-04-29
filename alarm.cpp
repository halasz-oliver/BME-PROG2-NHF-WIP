#include "alarm.h"

// Trigger komponens hozzárendelése
void alarm::setTrigger(component* c) {
    trigger = c;
}

// Aktív/inaktív állapot frissítése
void alarm::update() {
    if (trigger) {
        active = trigger->getState();
    }
}

// Riasztás állapotának lekérdezése
bool alarm::getState() const {
    return active;
}

#include "sensor.h"

// Érték beállítása
void Sensor::setValue(const int newValue) {
    currentValue = newValue;
}

// Küszöb ellenőrzése
bool Sensor::getState() const {
    return currentValue >= threshold;
}

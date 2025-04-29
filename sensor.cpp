#include "sensor.h"

// Aktuális érték beállítása
void Sensor::setValue(int newValue) {
    currentValue = newValue;
}

// Állapot lekérdezése küszöb alapján
bool Sensor::getState() const {
    return currentValue >= threshold;
}

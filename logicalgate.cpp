#include "logicalgate.h"

// Konstruktor/destruktor
LogicalGate::LogicalGate()
    : inputs(nullptr), inputCount(0), capacity(0) {}
LogicalGate::~LogicalGate() {
    delete[] inputs;
}

// Bemenet hozzáadás, kapacitás duplázódik ha kell
void LogicalGate::addInput(component* c) {
    if (inputCount == capacity) {
        const int newCap = (capacity == 0 ? 2 : capacity * 2);
        auto** temp = new component*[newCap];
        for (int i = 0; i < inputCount; ++i)
            temp[i] = inputs[i];
        delete[] inputs;
        inputs = temp;
        capacity = newCap;
    }
    inputs[inputCount++] = c;
}

// Minden bemenet törlése
void LogicalGate::clearInputs() {
    delete[] inputs;
    inputs = nullptr;
    inputCount = capacity = 0;
}

// Ellenőrzi, hogy adott komponens bemenet-e
bool LogicalGate::hasInput(component* c) const {
    for (int i = 0; i < inputCount; ++i)
        if (inputs[i] == c) return true;
    return false;
}

bool ANDGate::getState() const {
    if (inputCount == 0) return false;
    for (int i = 0; i < inputCount; ++i)
        if (!inputs[i]->getState())
            return false;
    return true;
}

bool ORGate::getState() const {
    if (inputCount == 0) return false;
    for (int i = 0; i < inputCount; ++i)
        if (inputs[i]->getState())
            return true;
    return false;
}

// NOT: csak egy bemenet
void NOTGate::addInput(component* c) {
    input = c;
}
void NOTGate::clearInputs() {
    input = nullptr;
}
bool NOTGate::getState() const {
    if (!input) return false;
    return !input->getState();
}

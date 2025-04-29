#include "logicalgate.h"

// Inicializálás
LogicalGate::LogicalGate()
    : inputs(nullptr), inputCount(0), capacity(0) { }

// Dinamikus tömb felszabadítása
LogicalGate::~LogicalGate() {
    delete[] inputs;
}

// Bemenet hozzáadása, kapacitás kétszerezése szükség esetén
void LogicalGate::addInput(component* c) {
    if (inputCount == capacity) {
        int newCap = (capacity == 0 ? 2 : capacity * 2);
        component** temp = new component*[newCap];
        for (int i = 0; i < inputCount; ++i)
            temp[i] = inputs[i];
        delete[] inputs;
        inputs = temp;
        capacity = newCap;
    }
    inputs[inputCount++] = c;
}

// AND: minden bemenetnek igaznak kell lennie
bool ANDGate::getState() const {
    if (inputCount == 0) return false;
    for (int i = 0; i < inputCount; ++i)
        if (!inputs[i]->getState())
            return false;
    return true;
}

// OR: legalább egy bemenetnek igaznak kell lennie
bool ORGate::getState() const {
    if (inputCount == 0) return false;
    for (int i = 0; i < inputCount; ++i)
        if (inputs[i]->getState())
            return true;
    return false;
}

// NOT: csak egy bemenet, megfordítja az értéket
void NOTGate::addInput(component* c) {
    input = c;
}

bool NOTGate::getState() const {
    if (!input) return false;
    return !input->getState();
}

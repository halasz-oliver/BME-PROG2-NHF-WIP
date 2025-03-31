#include "logicalgate.h"

LogicalGate::LogicalGate() {
    // Skeleton: Inicializálás
}

LogicalGate::~LogicalGate() {
    // Skeleton: Felszabadítás
}

void LogicalGate::addInput(component* c) {
    // Skeleton: Bemenet hozzáadása
}

bool ANDGate::getState() const {
    // Skeleton: AND logika megvalósítása
    return false;
}

bool ORGate::getState() const {
    // Skeleton: OR logika megvalósítása
    return false;
}

void NOTGate::addInput(component* c) {
    // Skeleton: NOT kapu esetén csak egy bemenet lehet
}

bool NOTGate::getState() const {
    // Skeleton: NOT logika megvalósítása
    return false;
}

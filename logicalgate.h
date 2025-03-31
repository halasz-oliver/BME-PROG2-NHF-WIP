#ifndef LOGICALGATE_H
#define LOGICALGATE_H

#include "Component.h"

// Absztrakt logikai kapu osztály
class LogicalGate : public component {
protected:
    // Dinamikus tömb használata STL konténer helyett
    component** inputs;
    int inputCount;
    int capacity;

public:
    LogicalGate();
    virtual ~LogicalGate();

    void addInput(component* c);
    virtual bool getState() const override = 0;
};

// Konkrét kapu típusok
class ANDGate : public LogicalGate {
public:
    ANDGate() : LogicalGate() {}
    bool getState() const override;
};

class ORGate : public LogicalGate {
public:
    ORGate() : LogicalGate() {}
    bool getState() const override;
};

class NOTGate : public LogicalGate {
private:
    // NOT kapunak csak egy bemenete lehet
    component* input;

public:
    NOTGate() : LogicalGate(), input(nullptr) {}

    // Felülírjuk az addInput függvényt, hogy csak egy bemenet lehessen
    void addInput(component* c);
    bool getState() const override;
};

#endif // LOGICALGATE_H

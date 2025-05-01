#ifndef LOGICALGATE_H
#define LOGICALGATE_H

#include "component.h"

// Absztrakt logikai kapu
class LogicalGate : public component {
protected:
    component** inputs;
    int inputCount;
    int capacity;

public:
    LogicalGate();
    ~LogicalGate() override;

    virtual void addInput(component* c);
    virtual void clearInputs();
    [[nodiscard]] bool hasInput(component* c) const;
    [[nodiscard]] bool getState() const override = 0;
};

class ANDGate final : public LogicalGate {
public:
    ANDGate() = default;
    [[nodiscard]] bool getState() const override;
};

class ORGate final : public LogicalGate {
public:
    ORGate() = default;
    [[nodiscard]] bool getState() const override;
};

class NOTGate final : public LogicalGate {
private:
    component* input;

public:
    NOTGate() : input(nullptr) {}
    void addInput(component* c) override;
    void clearInputs() override;
    [[nodiscard]] bool getState() const override;
};

#endif // LOGICALGATE_H

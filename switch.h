#ifndef SWITCH_H
#define SWITCH_H

#include "Component.h"

// Kapcsoló osztály
class Switch : public component {
private:
    bool isOn;

public:
    Switch(bool initialState = false) : isOn(initialState) {}

    void toggle();
    bool getState() const override;
};

#endif // SWITCH_H

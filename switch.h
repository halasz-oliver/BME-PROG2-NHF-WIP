#ifndef SWITCH_H
#define SWITCH_H

#include "component.h"

// Kapcsoló manuális állítással
class Switch final : public component {
private:
    bool isOn;

public:
    explicit Switch(const bool initialState = false) : isOn(initialState) {}
    void toggle();
    void setState(bool s);    // explicit beállítás
    [[nodiscard]] bool getState() const override;
};

#endif // SWITCH_H

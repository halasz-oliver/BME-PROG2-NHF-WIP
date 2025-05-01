#ifndef ALARM_H
#define ALARM_H

#include "component.h"

// Vészcsengő
class alarm final : public component {
private:
    bool active;
    component* trigger;

public:
    alarm() : active(false), trigger(nullptr) {}
    void setTrigger(component* c);
    void update();
    [[nodiscard]] bool getState() const override;
    [[nodiscard]] component* getTrigger() const; // ■ trigger lekérdezés
};

#endif // ALARM_H

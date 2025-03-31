#ifndef alarm_h
#define alarm_h

#include "component.h"

// Vészcsengő osztály
class alarm : public component {
private:
    bool active;
    component* trigger; // A komponens, ami aktiválja a vészcsengőt

public:
    alarm() : active(false), trigger(nullptr) {}

    void setTrigger(component* c);
    void update(); // Frissíti az állapotot a trigger alapján
    bool getState() const override;
};

#endif // alarm_h

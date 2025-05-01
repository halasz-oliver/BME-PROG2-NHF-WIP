#ifndef SENSOR_H
#define SENSOR_H

#include "component.h"

// Absztrakt szenzor
class Sensor : public component {
protected:
    int threshold;
    int currentValue;

public:
    explicit Sensor(const int thresh = 0) : threshold(thresh), currentValue(0) {}
    ~Sensor() override = default;

    void setValue(int newValue);
    [[nodiscard]] bool getState() const override;
};

// Konkrét szenzorok
class DiskCapacitySensor final : public Sensor {
public:
    explicit DiskCapacitySensor(const int thresh = 90) : Sensor(thresh) {}
};
class MemoryCapacitySensor final : public Sensor {
public:
    explicit MemoryCapacitySensor(const int thresh = 80) : Sensor(thresh) {}
};
class CpuLoadSensor final : public Sensor {
public:
    explicit CpuLoadSensor(const int thresh = 70) : Sensor(thresh) {}
};
class TemperatureSensor final : public Sensor {
public:
    explicit TemperatureSensor(const int thresh = 35) : Sensor(thresh) {}
};
class FireAlarm final : public Sensor {
public:
    FireAlarm() : Sensor(1) {}
};

#endif // SENSOR_H

#ifndef SENSOR_H
#define SENSOR_H

#include "component.h"

// Absztrakt Sensor osztály, amely az érzékelők alaptípusa
class Sensor : public component {
protected:
    int threshold;
    int currentValue;

public:
    Sensor(int thresh = 0) : threshold(thresh), currentValue(0) {}
    virtual ~Sensor() {}

    void setValue(int newValue);
    bool getState() const override;
};

// Különböző érzékelő típusok
class DiskCapacitySensor : public Sensor {
public:
    DiskCapacitySensor(int thresh = 90) : Sensor(thresh) {} // Alapértelmezett küszöbérték 90%
};

class MemoryCapacitySensor : public Sensor {
public:
    MemoryCapacitySensor(int thresh = 80) : Sensor(thresh) {} // Alapértelmezett küszöbérték 80%
};

class CpuLoadSensor : public Sensor {
public:
    CpuLoadSensor(int thresh = 70) : Sensor(thresh) {} // Alapértelmezett küszöbérték 70%
};

class TemperatureSensor : public Sensor {
public:
    TemperatureSensor(int thresh = 35) : Sensor(thresh) {} // Alapértelmezett küszöbérték 35°C
};

class FireAlarm : public Sensor {
public:
    FireAlarm() : Sensor(1) {} // Tűzjelző esetén 1-es érték jelenti a tüzet
};

#endif // SENSOR_H

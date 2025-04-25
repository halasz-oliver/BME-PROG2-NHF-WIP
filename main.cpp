#include <iostream>
#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"
#include "memtrace.h"

// Tesztprogram a rendszer komponenseinek bemutatására
int main() {
    // 1. Szenzorok létrehozása
    DiskCapacitySensor diskSensor(80);  // 80% küszöbérték
    MemoryCapacitySensor memSensor;
    CpuLoadSensor cpuSensor;
    TemperatureSensor tempSensor(30);   // 30°C küszöbérték
    FireAlarm fireAlarm;

    // 2. Értékek beállítása a szenzorokhoz (csak demonstrációs céllal)
    diskSensor.setValue(85);  // A diszk 85%-ban foglalt
    memSensor.setValue(70);   // A memória 70%-ban használt
    cpuSensor.setValue(90);   // A CPU 90%-ban terhelt
    tempSensor.setValue(32);  // A szerverszoba hőmérséklete 32°C
    fireAlarm.setValue(0);    // Nincs tűz

    // 3. Kapcsolók létrehozása
    Switch manualOverride(false);  // Kezdetben kikapcsolva
    Switch maintenanceMode(true);  // Kezdetben bekapcsolva

    // 4. Logikai kapuk létrehozása
    ANDGate criticalResourcesGate;
    criticalResourcesGate.addInput(&diskSensor);
    criticalResourcesGate.addInput(&memSensor);

    ORGate environmentalIssuesGate;
    environmentalIssuesGate.addInput(&tempSensor);
    environmentalIssuesGate.addInput(&fireAlarm);

    NOTGate maintenanceGate;
    maintenanceGate.addInput(&maintenanceMode);

    ANDGate finalAlarmTrigger;
    finalAlarmTrigger.addInput(&criticalResourcesGate);
    finalAlarmTrigger.addInput(&environmentalIssuesGate);
    finalAlarmTrigger.addInput(&maintenanceGate);
    finalAlarmTrigger.addInput(&manualOverride);

    // 5. Riasztás létrehozása
    alarm serverAlarm;
    serverAlarm.setTrigger(&finalAlarmTrigger);

    // 6. Rendszer állapotának ellenőrzése és megjelenítése
    serverAlarm.update();

    std::cout << "Rendszerállapot demonstrálása:" << std::endl;
    std::cout << "Diszk kapacitás riasztás: " << (diskSensor.getState() ? "AKTÍV" : "inaktív") << std::endl;
    std::cout << "Memória használat riasztás: " << (memSensor.getState() ? "AKTÍV" : "inaktív") << std::endl;
    std::cout << "CPU terhelés riasztás: " << (cpuSensor.getState() ? "AKTÍV" : "inaktív") << std::endl;
    std::cout << "Hőmérséklet riasztás: " << (tempSensor.getState() ? "AKTÍV" : "inaktív") << std::endl;
    std::cout << "Tűzriasztás: " << (fireAlarm.getState() ? "AKTÍV" : "inaktív") << std::endl;

    std::cout << "\nKapcsolók állapota:" << std::endl;
    std::cout << "Kézi felülbírálás: " << (manualOverride.getState() ? "BE" : "KI") << std::endl;
    std::cout << "Karbantartási mód: " << (maintenanceMode.getState() ? "BE" : "KI") << std::endl;

    std::cout << "\nLogikai kapuk állapota:" << std::endl;
    std::cout << "Erőforrás kritikus: " << (criticalResourcesGate.getState() ? "IGEN" : "NEM") << std::endl;
    std::cout << "Környezeti probléma: " << (environmentalIssuesGate.getState() ? "IGEN" : "NEM") << std::endl;
    std::cout << "Karbantartás?: " << (maintenanceGate.getState() ? "IGEN" : "NEM") << std::endl;
    std::cout << "Riasztás feltételek: " << (finalAlarmTrigger.getState() ? "TELJESÜLNEK" : "NEM TELJESÜLNEK") << std::endl;

    std::cout << "\nRiasztás állapota: " << (serverAlarm.getState() ? "AKTÍV" : "INAKTÍV") << std::endl;

    // 7. Szimuláljuk a kapcsoló állapotváltozását
    std::cout << "\nKapcsoló állapotának változtatása..." << std::endl;
    manualOverride.toggle();
    std::cout << "Kézi felülbírálás új állapota: " << (manualOverride.getState() ? "BE" : "KI") << std::endl;

    // 8. Frissítsük a rendszer állapotát
    serverAlarm.update();
    std::cout << "Riasztás új állapota: " << (serverAlarm.getState() ? "AKTÍV" : "INAKTÍV") << std::endl;

    return 0;
}

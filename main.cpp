#include <iostream>
#include <string>
#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"

int main() {
    component* comps[100] = { nullptr };  // minden komponens
    int compCount = 0;
    alarm* mainAlarm = nullptr;

    std::cout << "=== Szerverfarm felügyeleti CLI ===\n";
    std::cout << "Parancsok: help, list, create_sensor, create_switch, create_gate,\n";
    std::cout << "          create_alarm, connect, set, update, status, exit\n";

    while (true) {
        std::cout << "\n> ";
        std::string cmd;
        if (!(std::cin >> cmd)) break;

        if (cmd == "help") {
            std::cout << "\n=== ELÉRHETŐ PARANCSOK ===\n\n";

            std::cout << "[ Általános ]\n";
            std::cout << "  help                    - Parancsok listája\n";
            std::cout << "  list                    - Komponensek ID és típus szerinti listázása\n";
            std::cout << "  exit                    - Kilépés a programból\n\n";

            std::cout << "[ Komponens létrehozás ]\n";
            std::cout << "  create_sensor TYPE THRESHOLD\n";
            std::cout << "                          - Új szenzor létrehozása\n";
            std::cout << "                            TYPE: Disk | Mem | CPU | Temp | Fire\n";
            std::cout << "                            THRESHOLD: aktiválási küszöb (Fire esetén nem szükséges)\n";

            std::cout << "  create_switch STATE     - Új kapcsoló (STATE: 0 vagy 1)\n";
            std::cout << "  create_gate TYPE        - Új logikai kapu (TYPE: AND | OR | NOT)\n";
            std::cout << "  create_alarm            - Új vészcsengő létrehozása (egyetlen engedélyezett)\n\n";

            std::cout << "[ Kapcsolások ]\n";
            std::cout << "  connect SRC_ID DST_ID   - Kapcsolat létrehozása:\n";
            std::cout << "                            DST lehet logikai kapu vagy alarm\n";
            std::cout << "                            Kapuhoz: addInput(), Alarmhoz: setTrigger()\n\n";

            std::cout << "[ Működés ]\n";
            std::cout << "  set ID VALUE            - Szenzor aktuális értékének beállítása\n";
            std::cout << "  update                  - Fő riasztó állapotának frissítése\n";
            std::cout << "  status                  - Komponensek logikai állapotának kiírása\n";
        }
        else if (cmd == "list") {
            std::cout << "-- Komponensek listája --\n";
            for (int i = 0; i < compCount; ++i) {
                std::cout << "ID=" << i << "  : ";
                if (dynamic_cast<Sensor*>(comps[i])) {
                    std::cout << "Sensor";
                }
                else if (dynamic_cast<Switch*>(comps[i])) {
                    std::cout << "Switch";
                }
                else if (dynamic_cast<ANDGate*>(comps[i])) {
                    std::cout << "ANDGate";
                }
                else if (dynamic_cast<ORGate*>(comps[i])) {
                    std::cout << "ORGate";
                }
                else if (dynamic_cast<NOTGate*>(comps[i])) {
                    std::cout << "NOTGate";
                }
                else if (comps[i] == mainAlarm) {
                    std::cout << "Alarm";
                }
                else {
                    std::cout << "Ismeretlen";
                }
                std::cout << "\n";
            }
        }
        else if (cmd == "create_sensor") {
            std::string type; int thresh;
            std::cin >> type >> thresh;
            Sensor* s = nullptr;
            if (type == "Disk")  s = new DiskCapacitySensor(thresh);
            if (type == "Mem")   s = new MemoryCapacitySensor(thresh);
            if (type == "CPU")   s = new CpuLoadSensor(thresh);
            if (type == "Temp")  s = new TemperatureSensor(thresh);
            if (type == "Fire")  s = new FireAlarm();
            if (!s) {
                std::cout << "Hibás típus\n";
            } else {
                comps[compCount] = s;
                std::cout << "Szenzor létrehozva, ID=" << compCount << "\n";
                ++compCount;
            }
        }
        else if (cmd == "create_switch") {
            int st; std::cin >> st;
            auto* sw = new Switch(st != 0);
            comps[compCount] = sw;
            std::cout << "Kapcsoló létrehozva, ID=" << compCount
                      << " állapot=" << (sw->getState() ? "BE" : "KI") << "\n";
            ++compCount;
        }
        else if (cmd == "create_gate") {
            std::string type; std::cin >> type;
            component* g = nullptr;
            if (type == "AND") g = new ANDGate();
            if (type == "OR")  g = new ORGate();
            if (type == "NOT") g = new NOTGate();
            if (!g) {
                std::cout << "Hibás kaputípus\n";
            } else {
                comps[compCount] = g;
                std::cout << "Kapú létrehozva, ID=" << compCount << "\n";
                ++compCount;
            }
        }
        else if (cmd == "create_alarm") {
            if (mainAlarm) {
                std::cout << "Már van fő alarm (ID="
                          << (std::find(comps, comps+compCount, mainAlarm) - comps)
                          << ")\n";
            } else {
                mainAlarm = new alarm();
                comps[compCount] = mainAlarm;
                std::cout << "Fő alarm létrehozva, ID=" << compCount << "\n";
                ++compCount;
            }
        }
        else if (cmd == "connect") {
            int src, dst; std::cin >> src >> dst;
            if (src<0||src>=compCount||dst<0||dst>=compCount) {
                std::cout << "Hibás ID\n"; continue;
            }
            if (comps[dst] == mainAlarm) {
                mainAlarm->setTrigger(comps[src]);
                std::cout << "Alarm trigger beállítva (SRC="<<src<<")\n";
            } else if (auto lg = dynamic_cast<LogicalGate*>(comps[dst])) {
                lg->addInput(comps[src]);
                std::cout << "Bemenet hozzáadva kapuhoz (DST="<<dst<<")\n";
            } else {
                std::cout << "DST nem logikai kapu vagy alarm\n";
            }
        }
        else if (cmd == "set") {
            int id, val; std::cin >> id >> val;
            if (id<0||id>=compCount) { std::cout<<"Hibás ID\n"; continue; }
            if (auto s = dynamic_cast<Sensor*>(comps[id])) {
                s->setValue(val);
                std::cout << "Szenzor ID="<<id<<" érték="<<val<<"\n";
            } else {
                std::cout << "Ez nem szenzor\n";
            }
        }
        else if (cmd == "update") {
            if (mainAlarm) {
                mainAlarm->update();
                std::cout << "Alarm frissítve, állapot="
                          << (mainAlarm->getState() ? "AKTÍV" : "INAKTÍV") << "\n";
            } else {
                std::cout << "Nincs fő alarm\n";
            }
        }
        else if (cmd == "status") {
            std::cout << "-- Állapotok --\n";
            for (int i = 0; i < compCount; ++i) {
                std::cout << "ID="<<i<<" : ";
                if (auto s = dynamic_cast<Sensor*>(comps[i]))
                    std::cout<<(s->getState() ? "JELEZ" : "NEM JELEZ")<<"\n";
                else if (auto sw = dynamic_cast<Switch*>(comps[i]))
                    std::cout<<(sw->getState() ? "BE" : "KI")<<"\n";
                else if (auto g = dynamic_cast<ANDGate*>(comps[i]))
                    std::cout<<(g->getState() ? "IGEN" : "NEM")<<"\n";
                else if (auto g = dynamic_cast<ORGate*>(comps[i]))
                    std::cout<<(g->getState() ? "IGEN" : "NEM")<<"\n";
                else if (auto g = dynamic_cast<NOTGate*>(comps[i]))
                    std::cout<<(g->getState() ? "IGEN" : "NEM")<<"\n";
                else if (comps[i] == mainAlarm)
                    std::cout<<(mainAlarm->getState() ? "RIASZT" : "CSEND")<<"\n";
                else
                    std::cout<<"N/A\n";
            }
        }
        else if (cmd == "exit") {
            break;
        }
        else {
            std::cout << "Ismeretlen parancs ('help' a lista)\n";
        }
    }

    // memória felszabadítása
    for (int i = 0; i < compCount; ++i)
        delete comps[i];

    return 0;
}

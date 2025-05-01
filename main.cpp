#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <iomanip>
#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"

static component* comps[100] = { nullptr };
static int compCount = 0;
static std::mutex compMutex;
static std::atomic running{true};

// Háttér: másodpercenként frissítjük az összes alarmot
void periodicUpdater() {
    using namespace std::chrono_literals;
    while (running) {
        {
            std::lock_guard lock(compMutex);
            for (int i = 0; i < compCount; ++i)
                if (auto a = dynamic_cast<alarm*>(comps[i]))
                    a->update();
        }
        std::this_thread::sleep_for(1s);
    }
}

int main() {
    std::thread updater(periodicUpdater);

    std::cout << "=== Szerverfarm felügyeleti CLI ===\n"
              << "Parancsok: help, list, create_sensor, create_switch,\n"
              << " create_gate, create_alarm, connect, set, set_switch,\n"
              << " toggle, clear_inputs, remove, update, status, exit\n";

    while (true) {
        std::cout << "\n> ";
        std::string cmd;
        if (!(std::cin >> cmd)) break;

        if (cmd == "help") {
            std::cout << R"(
[Általános]
 help           - Parancsok listája
 list           - Komponensek listázása
 exit           - Kilépés

[Létrehozás]
 create_sensor TYPE THR  - Disk|Mem|CPU|Temp|Fire
 create_switch STATE     - 0=KI,1=BE
 create_gate TYPE        - AND|OR|NOT
 create_alarm            - Új riasztó

[Kapcsolás]
 connect SRC_ID DST_ID   - Átkötés

[Módosítás]
 set ID VALUE            - Szenzor érték
 set_switch ID VAL       - Kapcsoló explicit állap.
 toggle ID               - Kapcsoló fel-/lekapc.
 clear_inputs ID         - Kapu bemeneteinek törlése
 remove ID               - Komponens eltávolítása
 update                  - Riasztók frissítése

[Lekérdezés]
 status                  - Állapotok kiírása
)";
        }
        else if (cmd == "list") {
            std::lock_guard lock(compMutex);
            std::cout << "-- Komponensek --\n";
            for (int i = 0; i < compCount; ++i) {
                std::cout << "ID=" << std::setw(2) << i << " : ";
                if (dynamic_cast<Sensor*>(comps[i]))      std::cout << "Sensor";
                else if (dynamic_cast<Switch*>(comps[i])) std::cout << "Switch";
                else if (dynamic_cast<ANDGate*>(comps[i]))std::cout << "ANDGate";
                else if (dynamic_cast<ORGate*>(comps[i])) std::cout << "ORGate";
                else if (dynamic_cast<NOTGate*>(comps[i]))std::cout << "NOTGate";
                else if (dynamic_cast<alarm*>(comps[i]))  std::cout << "Alarm";
                else                                      std::cout << "Unknown";
                std::cout << "\n";
            }
        }
        else if (cmd == "create_sensor") {
            std::string type; int thr;
            std::cin >> type >> thr;
            Sensor* s = nullptr;
            if      (type=="Disk") s = new DiskCapacitySensor(thr);
            else if (type=="Mem")  s = new MemoryCapacitySensor(thr);
            else if (type=="CPU")  s = new CpuLoadSensor(thr);
            else if (type=="Temp") s = new TemperatureSensor(thr);
            else if (type=="Fire") s = new FireAlarm();
            if (!s) {
                std::cout << "Hibás típus! (Disk,Mem,CPU,Temp,Fire)\n";
            } else {
                std::lock_guard lock(compMutex);
                comps[compCount] = s;
                std::cout << "Sensor ID="<<compCount<<" létrehozva\n";
                ++compCount;
            }
        }
        else if (cmd == "create_switch") {
            int st; std::cin >> st;
            auto* sw = new Switch(st!=0);
            std::lock_guard lock(compMutex);
            comps[compCount] = sw;
            std::cout << "Switch ID="<<compCount
                      <<" állapot="<<(sw->getState()?"BE":"KI")<<"\n";
            ++compCount;
        }
        else if (cmd == "create_gate") {
            std::string type; std::cin >> type;
            component* g = nullptr;
            if      (type=="AND") g = new ANDGate();
            else if (type=="OR")  g = new ORGate();
            else if (type=="NOT") g = new NOTGate();
            if (!g) {
                std::cout << "Hibás kaputípus! (AND,OR,NOT)\n";
            } else {
                std::lock_guard lock(compMutex);
                comps[compCount] = g;
                std::cout << "Gate ID="<<compCount<<" létrehozva\n";
                ++compCount;
            }
        }
        else if (cmd == "create_alarm") {
            auto* a = new alarm();
            std::lock_guard lock(compMutex);
            comps[compCount] = a;
            std::cout << "Alarm ID="<<compCount<<" létrehozva\n";
            ++compCount;
        }
        else if (cmd == "connect") {
            int src,dst; std::cin>>src>>dst;
            std::lock_guard lock(compMutex);
            if (src<0||src>=compCount||dst<0||dst>=compCount) {
                std::cout<<"Érvénytelen ID\n"; continue;
            }
            if (auto a = dynamic_cast<alarm*>(comps[dst])) {
                a->setTrigger(comps[src]);
                std::cout<<"Alarm(ID="<<dst<<") trigger="<<src<<"\n";
            }
            else if (auto g = dynamic_cast<LogicalGate*>(comps[dst])) {
                g->addInput(comps[src]);
                std::cout<<"Kapcsolva: "<<src<<" -> Gate(ID="<<dst<<")\n";
            }
            else {
                std::cout<<"DST nem gate vagy alarm\n";
            }
        }
        else if (cmd == "set") {
            int id,val; std::cin>>id>>val;
            std::lock_guard lock(compMutex);
            if (id<0||id>=compCount) { std::cout<<"Érvénytelen ID\n"; continue; }
            if (auto s = dynamic_cast<Sensor*>(comps[id])) {
                s->setValue(val);
                std::cout<<"Sensor(ID="<<id<<") = "<<val<<"\n";
            } else {
                std::cout<<"Ez nem sensor\n";
            }
        }
        else if (cmd == "set_switch") {
            int id,val; std::cin>>id>>val;
            std::lock_guard lock(compMutex);
            if (id<0||id>=compCount||!dynamic_cast<Switch*>(comps[id])) {
                std::cout<<"Érvénytelen switch ID\n"; continue;
            }
            auto* sw = dynamic_cast<Switch*>(comps[id]);
            sw->setState(val!=0);
            std::cout<<"Switch(ID="<<id<<") = "<<(sw->getState()?"BE":"KI")<<"\n";
        }
        else if (cmd == "toggle") {
            int id; std::cin>>id;
            std::lock_guard lock(compMutex);
            if (id<0||id>=compCount||!dynamic_cast<Switch*>(comps[id])) {
                std::cout<<"Érvénytelen switch ID\n"; continue;
            }
            auto* sw = dynamic_cast<Switch*>(comps[id]);
            sw->toggle();
            std::cout<<"Switch(ID="<<id<<") = "<<(sw->getState()?"BE":"KI")<<"\n";
        }
        else if (cmd == "clear_inputs") {
            int id; std::cin>>id;
            std::lock_guard lock(compMutex);
            if (id<0||id>=compCount) { std::cout<<"Érvénytelen ID\n"; continue; }
            if (auto g = dynamic_cast<LogicalGate*>(comps[id])) {
                g->clearInputs();
                std::cout<<"Gate(ID="<<id<<") bemenetei törölve\n";
            } else {
                std::cout<<"Ez nem gate\n";
            }
        }
        else if (cmd == "remove") {
            int id; std::cin>>id;
            std::lock_guard lock(compMutex);
            if (id<0||id>=compCount) { std::cout<<"Érvénytelen ID\n"; continue; }
            bool used = false;
            for (int i = 0; i < compCount; ++i) {
                if (auto g = dynamic_cast<LogicalGate*>(comps[i])) {
                    if (g->hasInput(comps[id])) { used = true; break; }
                }
                if (auto a = dynamic_cast<alarm*>(comps[i])) {
                    if (a->getTrigger() == comps[id]) { used = true; break; }
                }
            }
            if (used) {
                std::cout<<"Előbb szüntesd meg minden hivatkozást!\n";
            } else {
                delete comps[id];
                for (int i = id; i < compCount-1; ++i)
                    comps[i] = comps[i+1];
                comps[--compCount] = nullptr;
                std::cout<<"Komponens törölve, ID="<<id<<"\n";
            }
        }
        else if (cmd == "update") {
            std::lock_guard lock(compMutex);
            for (int i = 0; i < compCount; ++i)
                if (auto a = dynamic_cast<alarm*>(comps[i]))
                    a->update();
            std::cout<<"Összes alarm frissítve\n";
        }
        else if (cmd == "status") {
            std::lock_guard lock(compMutex);
            std::cout<<"-- Állapotok --\n";
            for (int i = 0; i < compCount; ++i) {
                std::cout<< "ID="<< std::setw(2)<<i<<" : ";
                if (auto s = dynamic_cast<Sensor*>(comps[i])) {
                    std::cout<<(s->getState()?"JELEZ":"---");
                }
                else if (auto sw = dynamic_cast<Switch*>(comps[i])) {
                    std::cout<<(sw->getState()?"BE  ":"KI  ");
                }
                else if (auto g = dynamic_cast<ANDGate*>(comps[i])) {
                    std::cout<<(g->getState()?"IGEN":"NEM ");
                }
                else if (auto g = dynamic_cast<ORGate*>(comps[i])) {
                    std::cout<<(g->getState()?"IGEN":"NEM ");
                }
                else if (auto g = dynamic_cast<NOTGate*>(comps[i])) {
                    std::cout<<(g->getState()?"IGEN":"NEM ");
                }
                else if (auto a = dynamic_cast<alarm*>(comps[i])) {
                    if (a->getState())
                        std::cout<<"\033[31mRIASZT\033[0m";
                    else
                        std::cout<<"CSEND ";
                }
                else {
                    std::cout<<"N/A  ";
                }
                std::cout<<"\n";
            }
        }
        else if (cmd == "exit") {
            break;
        }
        else {
            std::cout<<"Ismeretlen parancs, help a listához\n";
        }
    }

    running = false;
    updater.join();
    for (int i = 0; i < compCount; ++i)
        delete comps[i];
    return 0;
}

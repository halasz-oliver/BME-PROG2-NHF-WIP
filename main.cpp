#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"

constexpr int MAX_COMPS = 100;
static component* comps[MAX_COMPS] = { nullptr }; // Fix méretű C-tömb
static int compCount = 0;                         // Aktuális komponensek száma

static std::mutex compMutex;
static std::atomic running{true};

// Segédfüggvény a komponensek típusának lekérdezésére
std::string getComponentType(const component* comp) {
    if (!comp) return "Invalid (null)";
    if (dynamic_cast<const DiskCapacitySensor*>(comp)) return "Sensor (Disk)";
    if (dynamic_cast<const MemoryCapacitySensor*>(comp)) return "Sensor (Mem)";
    if (dynamic_cast<const CpuLoadSensor*>(comp)) return "Sensor (CPU)";
    if (dynamic_cast<const TemperatureSensor*>(comp)) return "Sensor (Temp)";
    if (dynamic_cast<const FireAlarm*>(comp)) return "Sensor (Fire)";
    if (dynamic_cast<const Sensor*>(comp)) return "Sensor (Generic)";

    if (dynamic_cast<const Switch*>(comp)) return "Switch";
    if (dynamic_cast<const ANDGate*>(comp)) return "ANDGate";
    if (dynamic_cast<const ORGate*>(comp)) return "ORGate";
    if (dynamic_cast<const NOTGate*>(comp)) return "NOTGate";
    if (dynamic_cast<const alarm*>(comp)) return "Alarm";
    return "Unknown";
}

// Segédfüggvény a komponens lekérdezésére azonosító alapján
component* getComponentById(int id) {
    if (id >= 0 && id < compCount) {
        return comps[id];
    }
    return nullptr;
}

// Háttér: másodpercenként frissítjük az összes alarmot
void periodicUpdater() {
    using namespace std::chrono_literals;
    while (running) {
        { // Lock scope
            std::lock_guard lock(compMutex);
            for (int i = 0; i < compCount; ++i) {
                if (comps[i]) {
                    if (const auto a = dynamic_cast<alarm*>(comps[i])) {
                        a->update();
                    }
                }
            }
        }
        std::this_thread::sleep_for(1s);
    }
}

int main() {
    std::thread updater(periodicUpdater);

    std::cout << "=== Szerverfarm felügyeleti CLI ===\n"
              << "Üdvözöllek! Gépeld be a 'help' parancsot a részletes útmutatóért.\n";

    while (true) {
        std::cout << "\n> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd.empty()) continue;


        if (cmd == "help") {
            std::cout << R"(
================================ PARANCS ÚTMUTATÓ ================================

[ÁLTALÁNOS PARANCSOK]

  help
    Leírás: Megjeleníti ezt a súgó üzenetet, részletes leírással minden
              elérhető parancsról és annak használatáról.

  list
    Leírás: Kilistázza az összes jelenleg létező komponenst az egyedi
              azonosítójukkal (ID) és a típusukkal. Az ID-k a többi
              parancsban való hivatkozásra szolgálnak.
    Példa: list

  exit
    Leírás: Kilép a felügyeleti programból. A háttérben futó automatikus
              riasztófrissítést leállítja és a program által lefoglalt
              memóriát felszabadítja.
    Példa: exit

--------------------------------------------------------------------------------

[KOMPONENS LÉTREHOZÓ PARANCSOK]

  create_sensor TYPE KÜSZÖBÉRTÉK
    Leírás: Létrehoz egy új szenzort a megadott típussal és küszöbértékkel.
              Maximum 100 komponens hozható létre összesen.
    Paraméterek:
      TYPE: A szenzor típusa. Lehetséges értékek:
        - Disk: Diszkkapacitás szenzor (pl. %-os telítettség).
                Jelez, ha az értéke >= KÜSZÖBÉRTÉK.
        - Mem: Memóriakapacitás szenzor (pl. %-os foglaltság).
               Jelez, ha az értéke >= KÜSZÖBÉRTÉK.
        - CPU: CPU terheltség szenzor (pl. %-os terhelés).
               Jelez, ha az értéke >= KÜSZÖBÉRTÉK.
        - Temp: Szerverszoba hőmérséklet szenzor (pl. Celsius fok).
                Jelez, ha az értéke >= KÜSZÖBÉRTÉK.
        - Fire: Tűzjelző szenzor. Ennél a típusnál a KÜSZÖBÉRTÉK
                paramétert figyelmen kívül hagyja vagy nem kell megadni.
                A 'set <ID> 1' paranccsal aktiválható a jelzése.
      KÜSZÖBÉRTÉK: Egy egész szám, amely felett (vagy egyenlő) a szenzor
                   'IGAZ' állapotba kerül (jelez). 'Fire' esetén nem számít.
    Példák:
      create_sensor Temp 30
      create_sensor Disk 90
      create_sensor Fire

  create_switch KEZDETI_ÁLLAPOT
    Leírás: Létrehoz egy új manuálisan kapcsolható komponenst.
              Maximum 100 komponens hozható létre összesen.
    Paraméterek:
      KEZDETI_ÁLLAPOT: A kapcsoló kezdeti állapota.
        - 0: KI (logikai HAMIS)
        - 1: BE (logikai IGAZ)
    Példa:
      create_switch 1

  create_gate TYPE
    Leírás: Létrehoz egy új logikai kaput.
              Maximum 100 komponens hozható létre összesen.
    Paraméterek:
      TYPE: A kapu típusa. Lehetséges értékek:
        - AND: ÉS kapu.
        - OR:  VAGY kapu.
        - NOT: NEM kapu.
    Példák:
      create_gate AND

  create_alarm
    Leírás: Létrehoz egy új vészcsengőt (riasztót).
              Maximum 100 komponens hozható létre összesen.
    Példa: create_alarm

--------------------------------------------------------------------------------

[KOMPONENSEK ÖSSZEKAPCSOLÁSA]

  connect FORRÁS_ID CÉL_ID
    Leírás: Összeköt két komponenst.
    Paraméterek:
      FORRÁS_ID: A forrás komponens ID-ja.
      CÉL_ID:    A cél komponens ID-ja.
    Példa:
      connect 0 2

--------------------------------------------------------------------------------

[KOMPONENSEK MÓDOSÍTÁSA ÉS VEZÉRLÉSE]

  set SZENZOR_ID ÉRTÉK
    Leírás: Beállítja egy szenzor belső (mért) értékét.
    Paraméterek:
      SZENZOR_ID: A szenzor ID-ja.
      ÉRTÉK:      Az új érték.
    Példa:
      set 0 35

  set_switch KAPCSOLÓ_ID ÁLLAPOT
    Leírás: Explicit módon beállítja egy kapcsoló állapotát.
    Paraméterek:
      KAPCSOLÓ_ID: A kapcsoló ID-ja.
      ÁLLAPOT:     0 (KI) vagy 1 (BE).
    Példa:
      set_switch 1 0

  toggle KAPCSOLÓ_ID
    Leírás: Átbillenti egy kapcsoló állapotát.
    Paraméterek:
      KAPCSOLÓ_ID: A kapcsoló ID-ja.
    Példa:
      toggle 1

  clear_inputs KAPU_ID
    Leírás: Törli egy logikai kapu összes bemeneti kapcsolatát.
    Paraméterek:
      KAPU_ID: A logikai kapu ID-ja.
    Példa:
      clear_inputs 2

  remove KOMPONENS_ID
    Leírás: Eltávolít egy komponenst a rendszerből.
    Paraméterek:
      KOMPONENS_ID: Az eltávolítandó komponens ID-ja.
    Figyelem: Az ID-k eltolódhatnak a törlés után!
    Példa:
      remove 3

  update
    Leírás: Manuálisan frissíti az összes riasztó állapotát.
    Példa: update

--------------------------------------------------------------------------------

[ÁLLAPOT LEKÉRDEZÉSE]

  status
    Leírás: Kiírja az összes komponens aktuális állapotát.
    Példa: status

================================================================================
)";
        } else if (cmd == "list") {
            std::lock_guard lock(compMutex);
            std::cout << "-- Komponensek (" << compCount << "/" << MAX_COMPS << ") --\n";
            if (compCount == 0) {
                std::cout << "Nincsenek komponensek a rendszerben.\n";
            } else {
                for (int i = 0; i < compCount; ++i) {
                    std::cout << "ID=" << std::setw(2) << i << " : " << getComponentType(comps[i]) << "\n";
                }
            }
        } else if (cmd == "create_sensor") {
            if (compCount >= MAX_COMPS) {
                std::cout << "Hiba: Elérted a maximális komponensszámot (" << MAX_COMPS << ").\n";
                continue;
            }
            std::string type;
            int thr = 0;
            bool fireSensor = false;

            if (!(ss >> type)) {
                std::cout << "Hibás formátum. Használat: create_sensor TYPE [KÜSZÖBÉRTÉK]\n";
                continue;
            }

            if (type == "Fire") {
                fireSensor = true;
                std::string dummy_thr_check;
                if (ss >> dummy_thr_check) {
                     std::cout << "Figyelmeztetés: A 'Fire' szenzorhoz megadott küszöbérték ('" << dummy_thr_check << "') figyelmen kívül lesz hagyva.\n";
                }
            } else {
                 if (!(ss >> thr)) {
                    std::cout << "Hibás formátum: hiányzó KÜSZÖBÉRTÉK a(z) '" << type << "' szenzorhoz.\n";
                    continue;
                }
            }

            Sensor* s = nullptr;
            if (type == "Disk") s = new DiskCapacitySensor(thr);
            else if (type == "Mem") s = new MemoryCapacitySensor(thr);
            else if (type == "CPU") s = new CpuLoadSensor(thr);
            else if (type == "Temp") s = new TemperatureSensor(thr);
            else if (fireSensor) s = new FireAlarm();
            else {
                std::cout << "Hibás szenzor típus: '" << type << "'. Lehetségesek: Disk, Mem, CPU, Temp, Fire.\n";
                continue;
            }

            {
                 std::lock_guard lock(compMutex);
                 comps[compCount] = s;
                 std::cout << "Sensor létrehozva, ID=" << compCount << " (Típus: " << type;
                 if (!fireSensor) {
                     std::cout << ", Küszöb: " << thr;
                 }
                 std::cout << ")\n";
                 compCount++;
            }

        } else if (cmd == "create_switch") {
            if (compCount >= MAX_COMPS) {
                std::cout << "Hiba: Elérted a maximális komponensszámot (" << MAX_COMPS << ").\n";
                continue;
            }
            int st_val;
            if (!(ss >> st_val) || (st_val != 0 && st_val != 1) ) {
                std::cout << "Hibás formátum. Használat: create_switch KEZDETI_ÁLLAPOT (0 vagy 1)\n";
                continue;
            } {
                auto* sw = new Switch(st_val != 0);
                std::lock_guard lock(compMutex);
                comps[compCount] = sw;
                std::cout << "Switch létrehozva, ID=" << compCount << " (Kezdeti állapot: " << (sw->getState() ? "BE" : "KI") << ")\n";
                compCount++;
            }
        } else if (cmd == "create_gate") {
            if (compCount >= MAX_COMPS) {
                std::cout << "Hiba: Elérted a maximális komponensszámot (" << MAX_COMPS << ").\n";
                continue;
            }
            std::string type;
            if (!(ss >> type)) {
                std::cout << "Hibás formátum. Használat: create_gate TYPE\n";
                continue;
            }
            component* g = nullptr;
            if (type == "AND") g = new ANDGate();
            else if (type == "OR") g = new ORGate();
            else if (type == "NOT") g = new NOTGate();

            if (!g) {
                std::cout << "Hibás kaputípus: '" << type << "'. Lehetségesek: AND, OR, NOT.\n";
            } else {
                std::lock_guard lock(compMutex);
                comps[compCount] = g;
                std::cout << "Gate létrehozva, ID=" << compCount << " (Típus: " << type << ")\n";
                compCount++;
            }
        } else if (cmd == "create_alarm") {
            if (compCount >= MAX_COMPS) {
                std::cout << "Hiba: Elérted a maximális komponensszámot (" << MAX_COMPS << ").\n";
                continue;
            }
            auto* a = new alarm();
            {
                std::lock_guard lock(compMutex);
                comps[compCount] = a;
                std::cout << "Alarm létrehozva, ID=" << compCount << "\n";
                compCount++;
            }
        } else if (cmd == "connect") {
            int src_id, dst_id;
            if (!(ss >> src_id >> dst_id)) {
                std::cout << "Hibás formátum. Használat: connect FORRÁS_ID CÉL_ID\n";
                continue;
            }
            std::lock_guard lock(compMutex); // Lock a teljes műveletre
            component* srcComp = getComponentById(src_id);
            component* dstComp = getComponentById(dst_id);

            if (!srcComp) {
                std::cout << "Érvénytelen FORRÁS_ID: " << src_id << ".\n";
                continue;
            }
            if (!dstComp) {
                std::cout << "Érvénytelen CÉL_ID: " << dst_id << ".\n";
                continue;
            }

            if (auto a = dynamic_cast<alarm*>(dstComp)) {
                a->setTrigger(srcComp);
                std::cout << "Alarm(ID=" << dst_id << ") trigger komponense beállítva erre: "
                          << getComponentType(srcComp) << "(ID=" << src_id << ")\n";
            } else if (auto g = dynamic_cast<LogicalGate*>(dstComp)) {
                g->addInput(srcComp);
                std::cout << "Komponens " << getComponentType(srcComp) << "(ID=" << src_id
                          << ") rákötve: " << getComponentType(g) << "(ID=" << dst_id << ") bemenetére.\n";
            } else {
                std::cout << "A célkomponens (ID=" << dst_id << ", Típus: " << getComponentType(dstComp)
                          << ") nem fogad bemenetet (nem logikai kapu és nem riasztó).\n";
            }
        } else if (cmd == "set") {
            int id, val;
            if (!(ss >> id >> val)) {
                std::cout << "Hibás formátum. Használat: set SZENZOR_ID ÉRTÉK\n";
                continue;
            }
            std::lock_guard lock(compMutex);
            component* c = getComponentById(id);
            if (!c) { std::cout << "Érvénytelen ID: " << id << ".\n"; continue; }

            if (auto s = dynamic_cast<Sensor*>(c)) {
                s->setValue(val);
                std::cout << "Sensor(ID=" << id << ") értéke beállítva: " << val
                          << " (Új állapot: " << (s->getState() ? "JELEZ" : "NEM JELEZ") << ")\n";
            } else {
                std::cout << "A komponens (ID=" << id << ", Típus: " << getComponentType(c)
                          << ") nem Sensor, értéke nem állítható ezzel a paranccsal.\n";
            }
        } else if (cmd == "set_switch") {
            int id, st_val;
            if (!(ss >> id >> st_val) || (st_val != 0 && st_val != 1)) {
                std::cout << "Hibás formátum. Használat: set_switch KAPCSOLÓ_ID ÁLLAPOT (0 vagy 1)\n";
                continue;
            }
            std::lock_guard lock(compMutex);
            component* c = getComponentById(id);
             if (!c) { std::cout << "Érvénytelen ID: " << id << ".\n"; continue; }

            if (auto sw = dynamic_cast<Switch*>(c)) {
                sw->setState(st_val != 0);
                std::cout << "Switch(ID=" << id << ") állapota beállítva: "
                          << (sw->getState() ? "BE" : "KI") << "\n";
            } else {
                std::cout << "A komponens (ID=" << id << ", Típus: " << getComponentType(c)
                          << ") nem Switch.\n";
            }
        } else if (cmd == "toggle") {
            int id;
            if (!(ss >> id)) {
                std::cout << "Hibás formátum. Használat: toggle KAPCSOLÓ_ID\n";
                continue;
            }
            std::lock_guard lock(compMutex);
            component* c = getComponentById(id);
            if (!c) { std::cout << "Érvénytelen ID: " << id << ".\n"; continue; }

            if (auto sw = dynamic_cast<Switch*>(c)) {
                sw->toggle();
                std::cout << "Switch(ID=" << id << ") állapota átbillentve. Új állapot: "
                          << (sw->getState() ? "BE" : "KI") << "\n";
            } else {
                std::cout << "A komponens (ID=" << id << ", Típus: " << getComponentType(c)
                          << ") nem Switch.\n";
            }
        } else if (cmd == "clear_inputs") {
            int id;
            if (!(ss >> id)) {
                std::cout << "Hibás formátum. Használat: clear_inputs KAPU_ID\n";
                continue;
            }
            std::lock_guard lock(compMutex);
            component* c = getComponentById(id);
            if (!c) { std::cout << "Érvénytelen ID: " << id << ".\n"; continue; }

            if (auto g = dynamic_cast<LogicalGate*>(c)) {
                g->clearInputs();
                std::cout << getComponentType(g) << "(ID=" << id << ") bemenetei törölve.\n";
            } else {
                std::cout << "A komponens (ID=" << id << ", Típus: " << getComponentType(c)
                          << ") nem logikai kapu.\n";
            }
        } else if (cmd == "remove") {
            int id;
             if (!(ss >> id)) {
                std::cout << "Hibás formátum. Használat: remove KOMPONENS_ID\n";
                continue;
            }
            std::lock_guard lock(compMutex);
            component* compToRemove = getComponentById(id);

            if (!compToRemove) {
                std::cout << "Érvénytelen ID: " << id << " (lehet, hogy már törölték, vagy sosem létezett).\n";
                continue;
            }

            bool used = false;
            for (int i = 0; i < compCount; ++i) {
                if (comps[i] == compToRemove) continue;

                if (auto g = dynamic_cast<LogicalGate*>(comps[i])) {
                    if (g->hasInput(compToRemove)) {
                        std::cout << "Hiba: A(z) " << id << ". ID-jú komponenst (" << getComponentType(compToRemove)
                                  << ") bemenetként használja a(z) " << i << ". ID-jú " << getComponentType(g)
                                  << ". Előbb szüntesd meg a kapcsolatot.\n";
                        used = true;
                        break;
                    }
                }
                if (auto a = dynamic_cast<alarm*>(comps[i])) {
                    if (a->getTrigger() == compToRemove) {
                         std::cout << "Hiba: A(z) " << id << ". ID-jú komponenst (" << getComponentType(compToRemove)
                                  << ") triggerként használja a(z) " << i << ". ID-jú Alarm."
                                  << " Előbb változtasd meg a triggerét.\n";
                        used = true;
                        break;
                    }
                }
            }

            if (!used) {
                std::string typeName = getComponentType(comps[id]);
                delete comps[id];
                for (int i = id; i < compCount - 1; ++i) {
                    comps[i] = comps[i+1];
                }
                comps[compCount - 1] = nullptr;
                compCount--;

                std::cout << typeName << " (régi ID=" << id << ") törölve. Figyelem: Az ID-k eltolódhattak!\n";
            }
        } else if (cmd == "update") {
            { // Külön scope a locknak
                std::lock_guard lock(compMutex);
                for (int i = 0; i < compCount; ++i) {
                    if (comps[i]) {
                        if (auto a = dynamic_cast<alarm*>(comps[i])) {
                            a->update();
                        }
                    }
                }
            }
            std::cout << "Összes riasztó állapota manuálisan frissítve.\n";
        } else if (cmd == "status") {
            std::lock_guard lock(compMutex);
            std::cout << "-- Komponens Állapotok (" << compCount << " db) --\n";
             if (compCount == 0) {
                std::cout << "Nincsenek komponensek a rendszerben.\n";
            } else {
                std::cout << std::setw(5) << std::right << "ID" << " | "
                          << std::setw(18) << std::left << "Típus" << " | "
                          << "Állapot\n";
                std::cout << "-----|--------------------|----------------------\n";
                for (int i = 0; i < compCount; ++i) {
                    if (!comps[i]) continue; // Biztonsági ellenőrzés

                    component* comp = comps[i];
                    std::cout << std::setw(5) << std::right << i << " | "
                              << std::setw(18) << std::left << getComponentType(comp) << " | ";

                    std::string stateStr;
                    bool rawState = comp->getState();

                    if (dynamic_cast<const Sensor*>(comp)) {
                        stateStr = rawState ? "JELEZ" : "--- (OK)";
                    } else if (dynamic_cast<const Switch*>(comp)) {
                        stateStr = rawState ? "BE" : "KI";
                    } else if (dynamic_cast<const LogicalGate*>(comp)) {
                        stateStr = rawState ? "IGAZ (kimenet)" : "HAMIS (kimenet)";
                    } else if (auto a = dynamic_cast<const alarm*>(comp)) {
                        if (rawState) {
                             stateStr = "\033[31mRIASZT\033[0m";
                        } else {
                             stateStr = "CSENDES";
                        }
                    } else {
                        stateStr = rawState ? "IGAZ" : "HAMIS";
                    }
                    std::cout << stateStr << "\n";
                }
            }
        } else if (cmd == "exit") {
            std::cout << "Kilépés kérése...\n";
            break;
        } else {
            std::cout << "Ismeretlen parancs: '" << cmd << "'. Használd a 'help' parancsot a lehetséges parancsok listájáért.\n";
        }
    }

    running = false; // Leállítjuk a háttérszálat
    if (updater.joinable()) {
        updater.join();
    }


    // Dinamikusan foglalt memória felszabadítása
    {
        std::lock_guard lock(compMutex);
        for (int i = 0; i < compCount; ++i) {
            if (comps[i]) {
                delete comps[i];
                comps[i] = nullptr;
            }
        }
        compCount = 0;
    }

    std::cout << "A program sikeresen leállt. Minden erőforrás felszabadítva.\n";
    return 0;
}

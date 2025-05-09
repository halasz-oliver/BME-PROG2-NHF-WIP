# Szerverfarm Felügyeleti Rendszer (C++)

Ez a projekt egy egyszerű, parancssoros (CLI) alkalmazás, amely egy szerverfarm felügyeleti logikai rendszerét modellezi. Lehetővé teszi különféle szenzorok, kapcsolók, logikai kapuk (AND, OR, NOT) és riasztók (vészcsengők) létrehozását és összekapcsolását egy tetszőlegesen bonyolult logikai hálózattá.

A rendszer célja, hogy a szerverfarm kritikus állapotairól (pl. diszk kapacitás, memória telítettség, CPU terhelés, hőmérséklet, tűzjelzés) átfogó felügyeletet biztosítson és riasztásokat generáljon.

## Főbb jellemzők

*   **Moduláris komponensek:**
    *   **Szenzorok:** `DiskCapacitySensor`, `MemoryCapacitySensor`, `CpuLoadSensor`, `TemperatureSensor`, `FireAlarm`. Mindegyiknek saját küszöbértéke van (kivéve a tűzjelzőt), amely felett "jeleznek".
    *   **Switch:** Manuálisan kapcsolható (BE/KI) komponens.
    *   **Logikai kapuk:** `ANDGate`, `ORGate`, `NOTGate` a logikai műveletekhez. Tetszőleges számú bemenetet fogadhatnak (kivéve a NOT kaput, ami egyet).
    *   **Alarm:** Vészcsengő, amely egy másik komponens (trigger) állapotától függően aktiválódik.
*   **Parancssoros felület (CLI):** Parancsok a komponensek létrehozására, összekapcsolására, állapotuk módosítására és lekérdezésére.
*   **Dinamikus hálózatépítés:** A komponensek futás közben összekapcsolhatók, így komplex logikai feltételrendszerek alakíthatók ki.
*   **Automatikus riasztásfrissítés:** Egy háttérszál másodpercenként frissíti az összes riasztó állapotát a triggerük alapján.
*   **Memóriakezelés:** A projekt tartalmazza a `memtrace` modult a memóriaszivárgások felderítésére (a tesztelés során nem jelzett hibát).
*   **Egyszerű C tömb használata:** Nem használ STL tárolókat a komponensek tárolására, a feladatkiírásnak megfelelően egy fix méretű C stílusú tömböt (`component* comps[MAX_COMPS]`) alkalmaz.
*   **Tesztelés:** `gtest_lite` alapú unit tesztek biztosítják a komponensek és a rendszer logikájának helyes működését.

## Felépítés

A projekt a következő főbb fájlokból áll:

*   `main.cpp`: A parancssori felületet és a fő vezérlési logikát tartalmazza.
*   `component.h`: Az alap `component` interfész definíciója, amelyből minden más logikai elem származik.
*   `sensor.h` / `sensor.cpp`: Az absztrakt `Sensor` osztály és annak konkrét implementációi (Disk, Mem, CPU, Temp, Fire).
*   `switch.h` / `switch.cpp`: A `Switch` osztály implementációja.
*   `logicalgate.h` / `logicalgate.cpp`: Az absztrakt `LogicalGate` osztály és annak konkrét implementációi (AND, OR, NOT).
*   `alarm.h` / `alarm.cpp`: Az `alarm` (vészcsengő) osztály implementációja.
*   `test.cpp`: A `gtest_lite` unit teszteket tartalmazó fájl.
*   `memtrace.h` / `memtrace.cpp`: Memóriaszivárgás-detektor modul.
*   `gtest_lite.h`: Egyszerűsített tesztelési keretrendszer.

## Használat

### Fordítás

A program fordításához C++17 kompatibilis fordító szükséges (pl. g++). A `memtrace` modul használatához a `MEMTRACE` makrót definiálni kell fordításkor.

Példa fordítás g++-szal:
```
g++ -std=c++17 -DMEMTRACE main.cpp sensor.cpp switch.cpp logicalgate.cpp alarm.cpp memtrace.cpp -o server_monitor -pthread
```
A tesztek fordítása:
```
g++ -std=c++17 -DMEMTRACE test.cpp sensor.cpp switch.cpp logicalgate.cpp alarm.cpp memtrace.cpp -o run_tests -pthread
```

### Futtatás

A lefordított program indítása:
```
./server_monitor
```
A tesztek futtatása:
```
./run_tests
```

### Parancsok

A program indítása után a `>` promptnál adhatók ki a parancsok. A `help` parancs részletes útmutatót ad az összes elérhető parancsról.

**Főbb parancsok:**

*   `help`: Súgó megjelenítése.
*   `list`: Jelenlegi komponensek listázása ID-val és típussal.
*   `create_sensor TYPE KÜSZÖB`: Szenzor létrehozása (TYPE: `Disk`, `Mem`, `CPU`, `Temp`, `Fire`).
*   `create_switch 0|1`: Kapcsoló létrehozása kezdeti állapottal.
*   `create_gate AND|OR|NOT`: Logikai kapu létrehozása.
*   `create_alarm`: Riasztó létrehozása.
*   `connect FORRÁS_ID CÉL_ID`: Két komponens összekötése.
*   `set SZENZOR_ID ÉRTÉK`: Szenzor értékének beállítása.
*   `set_switch KAPCSOLÓ_ID 0|1`: Kapcsoló állapotának explicit beállítása.
*   `toggle KAPCSOLÓ_ID`: Kapcsoló állapotának átbillentése.
*   `clear_inputs KAPU_ID`: Logikai kapu bemeneteinek törlése.
*   `remove KOMPONENS_ID`: Komponens eltávolítása.
*   `update`: Riasztók manuális frissítése.
*   `status`: Összes komponens aktuális állapotának kiírása.
*   `exit`: Kilépés a programból.

## Mintahálózat példa

Egy egyszerű riasztási logika felépíthető például így:
1.  Hozz létre egy CPU szenzort: `create_sensor CPU 70` (ID=0)
2.  Hozz létre egy hőmérséklet szenzort: `create_sensor Temp 35` (ID=1)
3.  Hozz létre egy ÉS kaput: `create_gate AND` (ID=2)
4.  Hozz létre egy VAGY kaput: `create_gate OR` (ID=3)
5.  Hozz létre egy riasztót: `create_alarm` (ID=4)
6.  Kösd össze a CPU szenzort az ÉS kapuval: `connect 0 2`
7.  Kösd össze a hőmérséklet szenzort az ÉS kapuval: `connect 1 2`
    *Megjegyzés: Az ÉS kapu (ID=2) most akkor lesz IGAZ, ha a CPU > 70% ÉS a Temp > 35°C.*
8.  (Opcionális) Hozz létre egy manuális override kapcsolót: `create_switch 0` (ID=5)
9.  Kösd össze az ÉS kaput (ID=2) a VAGY kapuval (ID=3): `connect 2 3`
10. Kösd össze az override kapcsolót (ID=5) a VAGY kapuval (ID=3): `connect 5 3`
11. Kösd össze a VAGY kaput (ID=3) a riasztóval (ID=4): `connect 3 4`

Most a riasztó (ID=4) akkor fog jelezni, ha ((CPU terhelés magas ÉS hőmérséklet magas) VAGY az override kapcsoló be van kapcsolva).

Állapotok módosítása:
*   `set 0 75` (CPU terhelés beállítása 75%-ra)
*   `set 1 30` (Hőmérséklet beállítása 30°C-ra)
*   `toggle 5` (Override kapcsoló átbillentése)

Az állapotok ellenőrzése:
*   `status`
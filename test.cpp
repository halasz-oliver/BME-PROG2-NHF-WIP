#include <iostream>
#include "gtest_lite.h"
#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"

int main() {
    GTINIT(std::cin);  // Tesztelő környezet inicializálása

    // Sensor osztályok tesztjei
    TEST(Sensor, BasicFunctionality) {
        DiskCapacitySensor diskSensor(80);

        diskSensor.setValue(70);
        EXPECT_EQ(false, diskSensor.getState()) << "Az érzékelőnek inaktívnak kellene lennie 70% alatt";

        diskSensor.setValue(85);
        EXPECT_EQ(true, diskSensor.getState()) << "Az érzékelőnek aktívnak kellene lennie 80% felett";
    } ENDM

    // Switch osztály tesztjei
    TEST(Switch, ToggleFunctionality) {
        Switch sw(false);

        EXPECT_EQ(false, sw.getState()) << "Kezdeti állapot hibás";

        sw.toggle();
        EXPECT_EQ(true, sw.getState()) << "Toggle után az állapotnak változnia kellett volna";

        sw.toggle();
        EXPECT_EQ(false, sw.getState()) << "Második toggle után vissza kellett volna állnia";
    } ENDM

    // Logical Gate tesztek
    TEST(ANDGate, LogicTest) {
        ANDGate andGate;
        Switch sw1(true);
        Switch sw2(true);
        Switch sw3(false);

        andGate.addInput(&sw1);
        andGate.addInput(&sw2);

        EXPECT_EQ(true, andGate.getState()) << "AND kapunak igaznak kellene lennie, ha minden bemenet igaz";

        andGate.addInput(&sw3);
        EXPECT_EQ(false, andGate.getState()) << "AND kapunak hamisnak kellene lennie, ha bármely bemenet hamis";
    } ENDM

    TEST(ORGate, LogicTest) {
        ORGate orGate;
        Switch sw1(false);
        Switch sw2(false);

        orGate.addInput(&sw1);
        orGate.addInput(&sw2);

        EXPECT_EQ(false, orGate.getState()) << "OR kapunak hamisnak kellene lennie, ha minden bemenet hamis";

        sw1.toggle();  // true-ra állítjuk
        EXPECT_EQ(true, orGate.getState()) << "OR kapunak igaznak kellene lennie, ha bármely bemenet igaz";
    } ENDM

    TEST(NOTGate, LogicTest) {
        NOTGate notGate;
        Switch sw(true);

        notGate.addInput(&sw);
        EXPECT_EQ(false, notGate.getState()) << "NOT kapunak meg kellene fordítania a bemenet értékét";

        sw.toggle();  // false-ra állítjuk
        EXPECT_EQ(true, notGate.getState()) << "NOT kapunak meg kellene fordítania a bemenet értékét";
    } ENDM

    // Alarm tesztek
    TEST(Alarm, TriggerTest) {
        alarm alarm;
        Switch trigger(false);

        alarm.setTrigger(&trigger);
        alarm.update();
        EXPECT_EQ(false, alarm.getState()) << "A riasztásnak inaktívnak kellene lennie, ha a trigger inaktív";

        trigger.toggle();  // true-ra állítjuk
        alarm.update();
        EXPECT_EQ(true, alarm.getState()) << "A riasztásnak aktívnak kellene lennie, ha a trigger aktív";
    } ENDM

    // Komplex teszt a teljes rendszerre
    TEST(ComplexSystem, AlertCondition) {
        // Szenzorok
        TemperatureSensor tempSensor(30);
        FireAlarm fireAlarm;

        // Kapcsoló
        Switch maintenance(true);

        // Logikai kapuk
        ORGate envIssues;
        envIssues.addInput(&tempSensor);
        envIssues.addInput(&fireAlarm);

        NOTGate notMaintenance;
        notMaintenance.addInput(&maintenance);

        ANDGate alertCondition;
        alertCondition.addInput(&envIssues);
        alertCondition.addInput(&notMaintenance);

        // Riasztás
        alarm alarm;
        alarm.setTrigger(&alertCondition);

        // Teszt - karbantartási módban vagyunk, nem szabad riasztania
        tempSensor.setValue(35);  // Hőmérséklet túl magas
        alarm.update();
        EXPECT_EQ(false, alarm.getState()) << "Karbantartási módban nem szabadna riasztania";

        // Teszt - karbantartási módból kilépünk, most riasztania kell
        maintenance.toggle();  // Kikapcsoljuk a karbantartási módot
        alarm.update();
        EXPECT_EQ(true, alarm.getState()) << "Riasztania kellene a magas hőmérséklet miatt";

        // Teszt - ha a hőmérséklet normalizálódik, megszűnik a riasztás
        tempSensor.setValue(25);  // Normális hőmérséklet
        alarm.update();
        EXPECT_EQ(false, alarm.getState()) << "Normális hőmérsékletnél nem szabadna riasztania";
    } ENDM

    // Tesztek futtatása és eredmények kiértékelése
    return 0;
}

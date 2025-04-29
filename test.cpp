#include <iostream>
#include "gtest_lite.h"
#include "sensor.h"
#include "switch.h"
#include "logicalgate.h"
#include "alarm.h"

int main() {
    GTINIT(std::cin);

    // Szenzor működés: küszöb ellenőrzés
    TEST(Sensor, ValueAndThresholdLogic) {
        DiskCapacitySensor disk(80);
        disk.setValue(50);
        EXPECT_EQ(false, disk.getState()) << "Diszk 50% → nem kéne riasztani";

        disk.setValue(85);
        EXPECT_EQ(true, disk.getState()) << "Diszk 85% → riasztania kell";

        MemoryCapacitySensor mem(75);
        mem.setValue(74);
        EXPECT_EQ(false, mem.getState());

        mem.setValue(75);
        EXPECT_EQ(true, mem.getState());
    } ENDM

    // Switch alapműködés + kezdeti állapot
    TEST(Switch, ToggleFunctionality) {
        Switch sw(false);
        EXPECT_EQ(false, sw.getState()) << "Kapcsoló kezdetben KI";

        sw.toggle();
        EXPECT_EQ(true, sw.getState()) << "Kapcsoló BE";

        sw.toggle();
        EXPECT_EQ(false, sw.getState()) << "Kapcsoló vissza KI";

        Switch sw2(true);
        EXPECT_EQ(true, sw2.getState()) << "Kapcsoló kezdetben BE";
    } ENDM

    // ANDGate működésének teljes ellenőrzése
    TEST(ANDGate, LogicalOperation) {
        Switch s1(true), s2(true), s3(false);
        ANDGate gate;
        gate.addInput(&s1);
        gate.addInput(&s2);
        EXPECT_EQ(true, gate.getState()) << "Minden bemenet true → output is true";

        gate.addInput(&s3);  // false-t adunk hozzá
        EXPECT_EQ(false, gate.getState()) << "Egy false bemenet → output false";

        s3.toggle(); // true lesz
        EXPECT_EQ(true, gate.getState()) << "Most minden bemenet true";
    } ENDM

    // ORGate működésének teljes ellenőrzése
    TEST(ORGate, LogicalOperation) {
        Switch s1(false), s2(false);
        ORGate gate;
        gate.addInput(&s1);
        gate.addInput(&s2);
        EXPECT_EQ(false, gate.getState()) << "Minden bemenet false → output false";

        s1.toggle(); // true
        EXPECT_EQ(true, gate.getState()) << "Legalább egy bemenet true → output true";
    } ENDM

    // NOTGate működésének és input felülírásának tesztje
    TEST(NOTGate, SingleInputAndOverwrite) {
        Switch sw(true);
        NOTGate gate;
        gate.addInput(&sw);
        EXPECT_EQ(false, gate.getState()) << "Input true → output false";

        sw.toggle();  // false
        EXPECT_EQ(true, gate.getState()) << "Input false → output true";

        // Próbáljuk felülírni új inputtal
        Switch sw2(true);
        gate.addInput(&sw2);  // új bemenet
        EXPECT_EQ(false, gate.getState()) << "Új bemenet → új output érték";
    } ENDM

    // Alarm működés: trigger állapotának követése
    TEST(Alarm, RespondsToTriggerChange) {
        Switch manualTrigger(false);
        alarm a;
        a.setTrigger(&manualTrigger);

        a.update();
        EXPECT_EQ(false, a.getState()) << "Trigger false → alarm inaktív";

        manualTrigger.toggle();  // true
        a.update();
        EXPECT_EQ(true, a.getState()) << "Trigger true → alarm aktív";
    } ENDM

    // edge case: alarm setTrigger(nullptr)
    TEST(Alarm, NullTriggerSafe) {
        alarm a;
        a.setTrigger(nullptr);  // semmihez nincs csatlakoztatva
        a.update();
        EXPECT_EQ(false, a.getState()) << "Nincs trigger → mindig inaktív";
    } ENDM

    // Logikai hálózat teszt: több komponens együttműködése
    TEST(ComplexSystem, AlarmFromLogicalNetwork) {
        DiskCapacitySensor disk(80);
        TemperatureSensor temp(30);
        FireAlarm fire;
        Switch maintenance(true);
        Switch override(false);

        disk.setValue(90);  // true
        temp.setValue(32);  // true
        fire.setValue(0);   // false
        maintenance.toggle(); // off → NOT true
        override.toggle(); // toggle → true

        ANDGate critical;
        critical.addInput(&disk);

        ORGate environment;
        environment.addInput(&temp);
        environment.addInput(&fire);

        NOTGate notMaint;
        notMaint.addInput(&maintenance);

        ANDGate allConditions;
        allConditions.addInput(&critical);
        allConditions.addInput(&environment);
        allConditions.addInput(&notMaint);
        allConditions.addInput(&override);

        alarm a;
        a.setTrigger(&allConditions);
        a.update();

        EXPECT_EQ(true, a.getState()) << "Minden feltétel teljesül → riasztani kell";
    } ENDM

    return 0;
}

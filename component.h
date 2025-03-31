#ifndef COMPONENT_H
#define COMPONENT_H

// Absztrakt Component osztály, amely alapja minden rendszerkomponensnek
class component {
public:
    virtual ~component() {}

    // Minden komponens legfontosabb metódusa az állapot lekérdezése
    virtual bool getState() const = 0;
};

#endif // COMPONENT_H

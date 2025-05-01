#ifndef COMPONENT_H
#define COMPONENT_H

// Alap komponens interfész
class component {
public:
    virtual ~component() = default;
    [[nodiscard]] virtual bool getState() const = 0;
};

#endif // COMPONENT_H

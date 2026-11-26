#pragma once

#include <cstdint>
#include "position.h"

struct HealthChange {
    uint16_t previous;
    uint16_t current;
    Position pos;

    int delta() const { return static_cast<int>(current) - static_cast<int>(previous); }
};

class HealthListener {
public:
    virtual void on_damage(const HealthChange&) {}
    virtual void on_heal(const HealthChange&) {}
    virtual void on_death(const HealthChange&) {}
    virtual ~HealthListener() = default;
};

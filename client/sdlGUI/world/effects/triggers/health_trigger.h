#pragma once

#include <cstdint>
#include <optional>

#include "trigger.h"
#include "../listeners/health_listener.h"
#include "position.h"

class HealthTrigger : public Trigger<HealthListener> {
    std::optional<uint16_t> last_health;

public:
    void observe(uint16_t health, const Position& pos) {
        if (last_health && health != *last_health) {
            const HealthChange change{*last_health, health, pos};
            notify([&](HealthListener& l) {
                if (change.delta() < 0) {
                    l.on_damage(change);
                    if (health == 0) l.on_death(change);
                } else {
                    l.on_heal(change);
                }
            });
        }
        last_health = health;
    }
};

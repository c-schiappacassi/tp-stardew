#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

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

class HealthTrigger {
    std::vector<std::unique_ptr<HealthListener>> listeners;
    std::optional<uint16_t> last_health;

public:
    template <class T, class... Args>
    T& add(Args&&... args) {
        static_assert(std::is_base_of_v<HealthListener, T>, "listener must derive from HealthListener");
        auto owned = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *owned;
        listeners.push_back(std::move(owned));
        return ref;
    }

    void observe(uint16_t health, const Position& pos) {
        if (last_health && health != *last_health) {
            const HealthChange change{*last_health, health, pos};
            for (auto& l : listeners) {
                if (change.delta() < 0) {
                    l->on_damage(change);
                    if (health == 0) l->on_death(change);
                } else {
                    l->on_heal(change);
                }
            }
        }
        last_health = health;
    }
};
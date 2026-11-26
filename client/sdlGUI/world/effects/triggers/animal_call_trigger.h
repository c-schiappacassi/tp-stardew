#pragma once

#include <chrono>
#include <optional>
#include <random>

#include "trigger.h"
#include "../listeners/animal_call_listener.h"
#include "position.h"
#include "protocol_types.h"

class AnimalCallTrigger : public Trigger<AnimalCallListener> {
    using Clock = std::chrono::steady_clock;

    struct Gap {
        int min_seconds;
        int max_seconds;
    };

    static constexpr Gap COW_GAP{25, 70};
    static constexpr Gap CHICKEN_GAP{15, 45};

    std::mt19937 rng{std::random_device{}()};
    std::optional<Clock::time_point> next_call;

    Clock::time_point schedule(AnimalType type) {
        const Gap gap = is_cow(type) ? COW_GAP : CHICKEN_GAP;
        const int seconds = std::uniform_int_distribution<int>(gap.min_seconds, gap.max_seconds)(rng);
        return Clock::now() + std::chrono::seconds(seconds);
    }

public:
    static bool is_cow(AnimalType t) { return t == AnimalType::BrownCow || t == AnimalType::WhiteCow; }

    void observe(AnimalType type, const Position& pos) {
        if (!next_call) {
            next_call = schedule(type);
            return;
        }
        if (Clock::now() < *next_call) return;

        next_call = schedule(type);
        const AnimalCall call{type, pos};
        notify([&](AnimalCallListener& l) { l.on_call(call); });
    }
};

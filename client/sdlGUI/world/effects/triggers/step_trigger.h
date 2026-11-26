#pragma once

#include <chrono>
#include <optional>

#include "trigger.h"
#include "../listeners/step_listener.h"
#include "position.h"
#include "protocol_types.h"

class StepTrigger : public Trigger<StepListener> {
    using Clock = std::chrono::steady_clock;
    static constexpr std::chrono::milliseconds INTERVAL{400};

    std::optional<Clock::time_point> next_step;

public:
    void observe(PlayerState state, const Position& pos, Zone zone) {
        if (state != PlayerState::Walking) {
            next_step.reset();
            return;
        }
        const auto now = Clock::now();
        if (next_step && now < *next_step) return;

        next_step = now + INTERVAL;
        const Step step{pos, zone};
        notify([&](StepListener& l) { l.on_step(step); });
    }
};

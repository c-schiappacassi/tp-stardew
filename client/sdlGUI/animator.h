#pragma once

#include "utils.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

class Animator {
private:
    float elapsed_time = 0.0f;
    bool finished = false;

public:
    Animator() = default;

    void reset() {
        elapsed_time = 0.0f;
        finished = false;
    }

    void update(float delta_time) {
        elapsed_time += std::max(0.0f, delta_time);
    }

    bool is_finished() const {
        return finished;
    }

    Frame get_current_frame(const Clip& clip) {
        if (clip.sequence.empty()) {
            throw std::runtime_error("Clip sin frames");
        }

        if (clip.sequence.size() == 1) {
            return clip.sequence.front().frame;
        }

        int total_ms = 0;
        for (const auto& frame : clip.sequence) {
            total_ms += std::max(1, frame.duration_ms);
        }

        if (total_ms <= 0) {
            finished = true;
            return clip.sequence.back().frame;
        }

        float current_ms = elapsed_time * 1000.0f;

        if (clip.loop) {
            current_ms = std::fmod(
                current_ms, static_cast<float>(total_ms)
            );
        } else if (current_ms >= total_ms) {
            finished = true;
            return clip.sequence.back().frame;
        }

        for (const auto& frame : clip.sequence) {
            const int duration = std::max(1, frame.duration_ms);

            if (current_ms < duration) {
                return frame.frame;
            }

            current_ms -= duration;
        }

        return clip.sequence.back().frame;
    }
};
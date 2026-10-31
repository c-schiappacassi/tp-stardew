#pragma once

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <stdexcept>
#include "utils.h"

class Animator {
private:
    float elapsed_ms = 0.0f;
    bool finished = false;

    std::size_t current_frame_index(const Clip& clip) {
        if (clip.sequence.empty()) {
            return 0;
        }

        float elapsed = elapsed_ms;

        for (std::size_t i = 0; i < clip.sequence.size(); ++i) {
            const float duration = static_cast<float>(std::max(1, clip.sequence[i].duration_ms));

            if (elapsed < duration) {
                return i;
            }

            elapsed -= duration;
        }

        if (clip.loop) {
            float total_duration = 0.0f;

            for (const ClipFrame& frame: clip.sequence) {
                total_duration += static_cast<float>(std::max(1, frame.duration_ms));
            }

            if (total_duration > 0.0f) {
                elapsed_ms = std::fmod(elapsed_ms, total_duration);
            }

            return current_frame_index(clip);
        }

        finished = true;
        return clip.sequence.size() - 1;
    }

public:
    void reset() {
        elapsed_ms = 0.0f;
        finished = false;
    }

    void update(float delta_ms) {
        if (!finished && delta_ms > 0.0f) {
            elapsed_ms += delta_ms;
        }
    }

    bool is_finished() const { return finished; }

    const AnimatedFrame& get_current_frame(const Clip& clip) {
        if (clip.sequence.empty()) {
            throw std::runtime_error("Animator: clip sin frames");
        }

        const std::size_t index = current_frame_index(clip);
        return clip.sequence[index].frame;
    }
};
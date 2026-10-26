#pragma once

#include "utils.h"
#include <algorithm>

class Animator {
private:
    float elapsed_time;
    bool finished;

public:
    Animator() : elapsed_time(0.0f), finished(false) {}

    void reset() {
        elapsed_time = 0.0f;
        finished = false;
    }

    void update(float delta_time) {
        elapsed_time += delta_time;
    }

    bool is_finished() const {
        return finished;
    }

    Frame get_current_frame(const Clip& clip) {
        if (clip.fps <= 0) {
            return {clip.row, 0, clip.flip_h};
        }

        float frame_duration = 1.0f / static_cast<float>(clip.fps);
        int total_frames = clip.frames;

        int current_frame_index = static_cast<int>(elapsed_time / frame_duration);

        if (current_frame_index >= total_frames) {
            if (clip.loop) {
                elapsed_time = fmod(elapsed_time, total_frames * frame_duration);
                current_frame_index = static_cast<int>(elapsed_time / frame_duration);
            } else {
                current_frame_index = total_frames - 1;
                finished = true;
            }
        }

        return { clip.row, current_frame_index, clip.flip_h };
    }
};
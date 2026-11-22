#pragma once

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <optional>
#include <stdexcept>
#include "render_info.h"
#include "animator.h"
#include "utils.h"

#include "constants.h"

class ClipPlayer {
private:
    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;
    std::size_t index = 0;

public:
    const AnimatedFrame* frame(const RenderInfo& ctx, const ClipId& id) {
        if (!current_clip || !(*current_clip == id)) {
            current_clip = id;
            anim.reset();
        }
        if (last_it >= 0 && ctx.it > last_it) {
            anim.update(static_cast<float>(ctx.it - last_it) * TICK_MS);
        }
        last_it = ctx.it;

        if (!ctx.assets.has_clip(id)) {
            return nullptr;
        }
        const Clip& clip = ctx.assets.get_clip(id);
        if (clip.sequence.empty()) {
            return nullptr;
        }
        index = anim.get_current_frame_index(clip);
        return &clip.sequence[index].frame;
    }

    std::size_t frame_index() const { return index; }

    void reset() { current_clip.reset(); }
};
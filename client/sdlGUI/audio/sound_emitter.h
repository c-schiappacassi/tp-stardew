#pragma once

#include <cmath>
#include <vector>

#include "audio.h"
#include "../render_info.h"
#include "../utils.h"
#include "position.h"

class SoundEmitter {
private:
    static constexpr float FULL_VOLUME_RADIUS = 3.0f * TILE_TO_PIXEL;
    static constexpr float AUDIBLE_RADIUS = 22.0f * TILE_TO_PIXEL;

    struct Cue {
        SoundId id;
        Position pos;
    };

    std::vector<Cue> cues;

    static float distance_gain(const Position& ear, const Position& source) {
        const float dx = static_cast<float>(source.getX() - ear.getX());
        const float dy = static_cast<float>(source.getY() - ear.getY());
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= FULL_VOLUME_RADIUS) return 1.0f;
        if (distance >= AUDIBLE_RADIUS) return 0.0f;
        return 1.0f - (distance - FULL_VOLUME_RADIUS) / (AUDIBLE_RADIUS - FULL_VOLUME_RADIUS);
    }

public:
    SoundEmitter() = default;
    SoundEmitter(const SoundEmitter&) = delete;
    SoundEmitter& operator=(const SoundEmitter&) = delete;

    void cue(SoundId id, const Position& pos) { cues.push_back(Cue{id, pos}); }

    void clear() { cues.clear(); }

    void flush(const RenderInfo& ctx) {
        if (cues.empty()) return;
        if (ctx.audio.enabled()) {
            const Position& ear = ctx.state.get_local().pos;
            for (const Cue& c : cues) {
                const float gain = distance_gain(ear, c.pos);
                if (gain > 0.0f) ctx.audio.play(ctx.assets.get_sound(c.id), gain);
            }
        }
        cues.clear();
    }
};

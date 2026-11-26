#pragma once

#include "../listeners/health_listener.h"
#include "../../../audio/sound_emitter.h"

class OreHitSound : public HealthListener {
    SoundEmitter& sfx;

public:
    explicit OreHitSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_damage(const HealthChange& change) override {
        if (change.current > 0) sfx.cue(SoundId::OreHit, change.pos);
    }
    void on_death(const HealthChange& change) override { sfx.cue(SoundId::OreBreak, change.pos); }
};

#pragma once

#include "../listeners/health_listener.h"
#include "../../../audio/sound_emitter.h"

class PlayerHurtSound : public HealthListener {
    SoundEmitter& sfx;

public:
    explicit PlayerHurtSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_damage(const HealthChange& change) override { sfx.cue(SoundId::PlayerHurt, change.pos); }
};

#pragma once

#include "../listeners/frenzy_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class VampireFrenzySound : public FrenzyListener {
    SoundEmitter& sfx;

public:
    explicit VampireFrenzySound(SoundEmitter& sfx): sfx(sfx) {}

    void on_frenzy(const FrenzyStart& start) override {
        if (start.type == EnemyType::VampireMale || start.type == EnemyType::VampireFemale) {
            sfx.cue(SoundId::VampireFrenzy, start.pos);
        }
    }
};

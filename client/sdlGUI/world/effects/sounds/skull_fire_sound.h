#pragma once

#include "../listeners/attack_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class SkullFireSound : public AttackListener {
    SoundEmitter& sfx;

public:
    explicit SkullFireSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_attack(const AttackStart& start) override {
        if (start.type == EnemyType::Skull) sfx.cue(SoundId::SkullFire, start.pos);
    }
};

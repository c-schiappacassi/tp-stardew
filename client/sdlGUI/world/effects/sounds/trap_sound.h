#pragma once

#include "../listeners/trap_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class TrapSound : public TrapListener {
    SoundEmitter& sfx;

public:
    explicit TrapSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_activated(const TrapActivation& activation) override {
        sfx.cue(activation.type == TrapType::WallFlame ? SoundId::TrapFlame : SoundId::TrapSpikes, activation.pos);
    }
};

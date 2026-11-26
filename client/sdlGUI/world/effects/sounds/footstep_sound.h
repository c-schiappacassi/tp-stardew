#pragma once

#include "../listeners/step_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class FootstepSound : public StepListener {
    SoundEmitter& sfx;

    static SoundId sound_for(Zone zone) {
        switch (zone) {
            case Zone::Dungeon: return SoundId::StepStone;
            case Zone::InsideHouse: return SoundId::StepWood;
            default: return SoundId::StepGrass;  // Farm y Town
        }
    }

public:
    explicit FootstepSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_step(const Step& step) override { sfx.cue(sound_for(step.zone), step.pos); }
};

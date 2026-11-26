#pragma once

#include "../listeners/soil_listener.h"
#include "../../../audio/sound_emitter.h"

class TilledSound : public SoilListener {
    SoundEmitter& sfx;

public:
    explicit TilledSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_tilled(const SoilChange& change) override { sfx.cue(SoundId::SoilTilled, change.pos); }
};

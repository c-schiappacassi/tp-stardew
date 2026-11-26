#pragma once

#include "../listeners/chest_listener.h"
#include "../../../audio/sound_emitter.h"

class ChestSound : public ChestListener {
    SoundEmitter& sfx;

public:
    explicit ChestSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_opened(const ChestOpening& chest) override { sfx.cue(SoundId::ChestOpen, chest.pos); }
    void on_closed(const ChestOpening& chest) override { sfx.cue(SoundId::ChestClose, chest.pos); }
};

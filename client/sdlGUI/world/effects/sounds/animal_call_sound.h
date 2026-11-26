#pragma once
#include "../listeners/animal_call_listener.h"
#include "../triggers/animal_call_trigger.h"
#include "../../../audio/sound_emitter.h"


class AnimalCallSound : public AnimalCallListener {
    SoundEmitter& sfx;

public:
    explicit AnimalCallSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_call(const AnimalCall& call) override {
        sfx.cue(AnimalCallTrigger::is_cow(call.type) ? SoundId::CowMoo : SoundId::ChickenCall, call.pos);
    }
};

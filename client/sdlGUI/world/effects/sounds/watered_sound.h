#pragma once

#include "../listeners/crop_listener.h"
#include "../../../audio/sound_emitter.h"

class WateredSound : public CropListener {
    SoundEmitter& sfx;

public:
    explicit WateredSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_watered(const CropWatering& watering) override { sfx.cue(SoundId::PlantWatered, watering.pos); }
};

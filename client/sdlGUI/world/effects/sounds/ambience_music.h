#pragma once

#include <optional>

#include "../listeners/zone_listener.h"
#include "../../../asset_handling/asset_manager.h"
#include "../../../audio/audio.h"

#include "protocol_types.h"

class AmbienceMusic : public ZoneListener {
    static constexpr int FADE_MS = 1500;

    Audio& audio;
    const AssetManager& assets;
    std::optional<SoundId> playing;

    static std::optional<SoundId> track_for(Zone zone) {
        switch (zone) {
            case Zone::Farm: return SoundId::AmbienceFarm;
            case Zone::Town: return SoundId::AmbienceTown;
            case Zone::Dungeon: return SoundId::AmbienceDungeon;
            default: return std::nullopt;
        }
    }

public:
    AmbienceMusic(Audio& audio, const AssetManager& assets): audio(audio), assets(assets) {}

    void on_enter(const ZoneChange& change) override {
        std::optional<SoundId> track = track_for(change.current);
        if (!track && !playing) track = SoundId::AmbienceFarm;
        if (!track || track == playing) return;

        playing = track;
        audio.play_ambience(assets.get_sound(*track), FADE_MS);
    }
};

#pragma once

#include <random>

#include "../utils.h"

namespace audio_volumes {
    static constexpr int MAX_LEVEL = 10;
    static constexpr int DEFAULT_LEVEL = 8;
}


class Audio {
private:
    bool ready = false;
    int music = audio_volumes::DEFAULT_LEVEL;
    int sfx = audio_volumes::DEFAULT_LEVEL;
    int ambience_channel = -1;
    float ambience_gain = 1.0f;

    int free_channel() const;
    static float level_gain(int level) { return static_cast<float>(level) / audio_volumes::MAX_LEVEL; }

public:
    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    bool enabled() const { return ready; }
    
    int music_level() const { return music; }
    int sfx_level() const { return sfx; }
    void set_music_level(int level);
    void set_sfx_level(int level);

    void play(const Sound& sound, float volume = 1.0f);

    void play_ambience(const Sound& sound, int fade_ms);
};

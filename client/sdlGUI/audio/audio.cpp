#include "audio.h"

#include <algorithm>
#include <iostream>

#include <SDL.h>
#include <SDL_mixer.h>

namespace {
    constexpr int FREQUENCY = 44100;
    constexpr int BUFFER_SAMPLES = 2048;

    constexpr int TOTAL_CHANNELS = 32;
    constexpr int AMBIENCE_CHANNELS = 2;

    int to_mix_volume(float v) {
        return std::clamp(static_cast<int>(v * MIX_MAX_VOLUME + 0.5f), 0, MIX_MAX_VOLUME);
    }
}

Audio::Audio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::cerr << "[Audio] No se pudo iniciar el audio, el juego va a estar en silencio: " << SDL_GetError() << '\n';
        return;
    }
    if (Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, 2, BUFFER_SAMPLES) != 0) {
        std::cerr << "[Audio] No se pudo abrir el dispositivo, el juego va a estar en silencio: " << Mix_GetError() << '\n';
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }
    Mix_AllocateChannels(TOTAL_CHANNELS);
    Mix_ReserveChannels(AMBIENCE_CHANNELS);
    ready = true;
}

Audio::~Audio() {
    if (!ready) return;
    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int Audio::free_channel() const {
    for (int ch = AMBIENCE_CHANNELS; ch < TOTAL_CHANNELS; ++ch) {
        if (!Mix_Playing(ch)) return ch;
    }
    return -1;
}

void Audio::set_music_level(int level) {
    music = std::clamp(level, 0, audio_volumes::MAX_LEVEL);
    if (ready && ambience_channel >= 0) {
        Mix_Volume(ambience_channel, to_mix_volume(ambience_gain * level_gain(music)));
    }
}

void Audio::set_sfx_level(int level) { sfx = std::clamp(level, 0, audio_volumes::MAX_LEVEL); }

void Audio::play(const Sound& sound, float volume) {
    if (!ready || !sound.chunk || volume <= 0.0f || sfx == 0) return;

    const int channel = free_channel();
    if (channel < 0) return;

    Mix_Volume(channel, to_mix_volume(volume * sound.gain * level_gain(sfx)));
    Mix_PlayChannel(channel, sound.chunk.get(), 0);
}

void Audio::play_ambience(const Sound& sound, int fade_ms) {
    if (!ready || !sound.chunk) return;

    if (ambience_channel >= 0) Mix_FadeOutChannel(ambience_channel, fade_ms);
    ambience_channel = (ambience_channel + 1) % AMBIENCE_CHANNELS;
    ambience_gain = sound.gain;
    
    Mix_Volume(ambience_channel, to_mix_volume(ambience_gain * level_gain(music)));
    Mix_FadeInChannel(ambience_channel, sound.chunk.get(), -1, fade_ms);
}

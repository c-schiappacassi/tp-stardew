#pragma once
#include <filesystem>
#include <map>
#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <utility>
#include <SDL2pp/SDL2pp.hh>
#include "utils.h"
#include "protocol_types.h"
#include <SDL_image.h>


class AssetManager {
private:
    // todos estos mapas se hardcodean en el constructor de assetmanager
    std::map<SpriteId, Sprite> sprites;
    std::map<ClipId, Clip> clips;
    std::map<SoundId, Sound> sounds;

    void load_farmer_clips();
    void load_sprites(SDL2pp::Renderer& renderer);
public:
    explicit AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& /*assets_root*/){
        load_farmer_clips();
        load_sprites(renderer);
    }

    Sprite& get_sprite(SpriteId id) { return sprites.at(id); }
    const Sound& get_sound(SoundId id) const { return sounds.at(id); }
    const Clip& get_clip(const ClipId& id) const { return clips.at(id); }
};
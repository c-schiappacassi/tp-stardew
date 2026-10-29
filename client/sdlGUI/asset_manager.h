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
    std::map<ItemId, SpriteId> item_id_to_sprite;
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

    // TODO: implementar esta func y la carga
    Sprite& get_sprite(ItemId id) { return sprites.at(item_id_to_sprite.at(id)); }
};

// TODO: TBD de usar esto para lo de los anchos y altos de las cosas:
/*
struct SpriteRegion {      // un pedazo de una hoja
    SpriteId sheet;
    SDL2pp::Rect src;      // en px de la textura
};
std::map<ItemId, SpriteRegion> item_regions;   // en vez de map<ItemId, SpriteId>

const SpriteRegion& get_item_region(ItemId id) const { return item_regions.at(id); }
*/
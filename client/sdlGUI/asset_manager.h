#pragma once

#include <filesystem>
#include <map>
#include <stdexcept>
#include <utility>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_image.h>

#include "protocol_types.h"
#include "utils.h"

class AssetManager {
private:
    std::map<SpriteId, SDL2pp::Texture> textures;
    std::map<ClipId, Clip> clips;
    std::map<ItemId, Sprite> item_sprites;
    std::map<std::pair<ResourceType, ResourceState>, Sprite> ore_sprites;
    std::map<SoundId, Sound> sounds;

    SDL2pp::Texture load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path);
    void load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);
    void load_farmer_clips();
    void load_ore_sprites();
    void load_item_sprites();

public:
    AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);

    SDL2pp::Texture& get_texture(SpriteId id);
    const Clip& get_clip(const ClipId& id) const;

    const Sprite& get_sprite(ItemId id) const;
    const Sprite* find_sprite(ItemId id) const;

    const Sprite& get_sprite(ResourceType type, ResourceState state) const;
    const Sound& get_sound(SoundId id) const;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};
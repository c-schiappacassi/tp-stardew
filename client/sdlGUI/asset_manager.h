#pragma once

#include <filesystem>
#include <map>
#include <stdexcept>
#include <utility>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_image.h>
#include <vector>

#include "protocol_types.h"
#include "utils.h"

class AssetManager {
private:
    std::map<SpriteId, SDL2pp::Texture> textures;
    std::map<ClipId, Clip> clips;
    std::map<ItemId, Sprite> item_sprites;
    std::map<AnimalType, SpriteId> animal_sprite_ids;
    std::map<UiSpriteId, Sprite> ui_sprites;
    std::map<std::pair<CropType, CropState>, Sprite> crop_sprites;
    std::map<std::pair<ResourceType, ResourceState>, Sprite> ore_sprites;
    std::map<SoundId, Sound> sounds;
    std::map<std::pair<ItemId, Direction>, std::vector<ToolFrame>> tool_frames;
    std::map<TileTextureId, Sprite> tile_sprites;
    std::map<DecorId, Sprite> decor_sprites;
    std::map<BuildingId, Sprite> building_sprites;
    std::map<std::pair<FurnitureId, Direction>, FlippableSprite> furniture_sprites;
    std::map<EmoteId, Sprite> emote_sprites;
    std::map<PortalId, Sprite> portal_sprites;
    std::map<InteriorId, Sprite> interior_sprites;
    std::map<NpcId, Sprite> npc_sprites;
    std::map<Quality, Sprite> quality_sprites;

    SDL2pp::Texture load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path);
    void load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);
    void load_farmer_clips();
    void load_ore_sprites();
    void load_item_sprites();
    void load_ui_sprites();
    void load_crop_sprites();
    void load_cow_clips();
    void load_chicken_clips();
    void load_tool_frames();
    void load_decor_sprites();
    void load_tile_sprites();
    void load_furniture_sprites();
    void load_building_sprites();
    void load_skeleton_clips();
    void load_golem_clips();
    void load_bat_clips();
    void load_skull_clips();
    void load_trap_clips();
    void load_projectile_clips();
    void load_animal_ids();
    void load_emotes();
    void load_portals();
    void load_interiors();
    void load_npcs();
    void load_qualitys();

public:
    AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);

    SDL2pp::Texture& get_texture(SpriteId id);
    const Clip& get_clip(const ClipId& id) const;

    const Sprite& get_sprite(ItemId id) const;
    const Sprite* find_sprite(ItemId id) const;

    const Sprite& get_sprite(UiSpriteId id) const;
    const Sprite& get_sprite(TileTextureId id) const;
    const Sprite& get_sprite(DecorId id) const;
    const Sprite& get_sprite(BuildingId id) const;
    const FlippableSprite& get_sprite(FurnitureId id, Direction dir) const;
    const Sprite& get_sprite(EmoteId id) const;
    const Sprite& get_sprite(CropType type, CropState state) const;
    const Sprite& get_sprite(ResourceType type, ResourceState state) const;
    const Sprite& get_sprite(PortalId id) const;
    const Sprite& get_sprite(InteriorId id) const;
    const Sprite& get_sprite(NpcId id) const;
    const Sprite& get_sprite(Quality type) const;

    const SpriteId& get_sprite_id(AnimalType type) const;
    const ToolFrame* find_tool_frame(ItemId item, Direction dir, std::size_t frame_index) const;

    // TODO: Sounds
    const Sound& get_sound(SoundId id) const;

    bool has_clip(const ClipId& id) const { return clips.count(id) > 0; }

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};
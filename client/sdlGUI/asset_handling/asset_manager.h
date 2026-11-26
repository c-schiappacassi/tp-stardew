#pragma once

#include <filesystem>
#include <map>
#include <stdexcept>
#include <utility>

#include <SDL2pp/SDL2pp.hh>
#include <vector>

#include "asset_tables.h"
#include "protocol_types.h"
#include "../utils.h"

namespace placeholders {
    constexpr SpriteId PLACEHOLDER_SPRITE_ID = SpriteId::PlaceHolder;
    constexpr Sprite PLACEHOLDER_SPRITE = {PLACEHOLDER_SPRITE_ID , 0 , 0, 16, 16 };
    constexpr FlippableSprite PLACEHOLDER_FLIPPABLESPRITE = {PLACEHOLDER_SPRITE, false};
    constexpr ToolFrame PLACEHOLDER_TOOLFRAME = {PLACEHOLDER_SPRITE, 0, SDL2pp::Point{0,0}};
    inline const Clip PLACEHOLDER_CLIP = { { { {PLACEHOLDER_SPRITE, false, {}}, 100 } }, false };
    inline const Sound PLACEHOLDER_SOUND{};
}

class AssetManager {
private:
    AssetTables tables;

public:
    AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);

    bool has_clip(const ClipId& id) const { return tables.clips.count(id) > 0; }

    SDL2pp::Texture& get_texture(SpriteId id);

    const SpriteId& get_sprite_id(AnimalType type) const;
    const SpriteId& get_sprite_id(EnemyType type, bool frenzied) const;
    const SpriteId& get_sprite_id(TrapType type) const;

    const Clip& get_clip(const ClipId& id) const;

    const ToolFrame& get_tool_frame(ItemId item, Direction dir, std::size_t frame_index) const;

    const int& get_height(EnemyType type) const;
    const int& get_height(TrapType type) const;
    const int& get_depth_offset(TrapType type) const;
    Direction get_clip_dir(TrapType type, Direction facing) const;

    const Sprite& get_sprite(ItemId id) const;
    const Sprite& get_sprite(UiSpriteId id) const;
    const Sprite& get_sprite(TileTextureId id) const;
    const Sprite& get_sprite(DecorId id) const;
    const Sprite& get_sprite(BuildingId id) const;
    const Sprite& get_sprite(FramePieceId id) const;
 
    const Sprite& get_tile_sprite(TileTextureId id, int tile_x, int tile_y) const;
    const Sprite& get_sprite(EmoteId id) const;
    const Sprite& get_sprite(CropType type, CropState state) const;
    const Sprite& get_sprite(ResourceType type, ResourceState state) const;
    const Sprite& get_sprite(PortalId id) const;
    const Sprite& get_sprite(InteriorId id) const;
    const Sprite& get_sprite(NpcId id) const;
    const Sprite& get_sprite(Quality type) const;
    const FlippableSprite& get_sprite(FurnitureId id, Direction dir) const;

    const Sound& get_sound(SoundId id) const;


    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
};
#pragma once

#include <map>
#include <utility>
#include <vector>

#include <SDL2pp/SDL2pp.hh>

#include "protocol_types.h"
#include "../utils.h"

struct EnemyVisual {
    SpriteId sprite;
    SpriteId frenzy_sprite;
    int cell_h;
};
struct TrapVisual {
    SpriteId sprite;
    int cell_h;
    int depth_offset;
    bool directional;
};
struct AssetTables {
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
    std::map<EnemyType, EnemyVisual> enemy_visuals;
    std::map<TrapType, TrapVisual> trap_visuals;
};

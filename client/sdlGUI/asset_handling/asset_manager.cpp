#include "asset_manager.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <SDL_image.h>
#include <SDL_mixer.h>

#include "dtos.h"
#include "sound_defs.h"

using namespace SpriteDefs;

namespace {
    const EnemyVisual MISSING_ENEMY_VISUAL{ placeholders::PLACEHOLDER_SPRITE_ID, placeholders::PLACEHOLDER_SPRITE_ID, 32 };
    const TrapVisual MISSING_TRAP_VISUAL{ placeholders::PLACEHOLDER_SPRITE_ID, 16, 0, true };

    template <class E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
    std::string describe(E e) {
        return std::to_string(static_cast<long long>(e));
    }

    template <class A, class B>
    std::string describe(const std::pair<A, B>& p) {
        return "(" + describe(p.first) + ", " + describe(p.second) + ")";
    }

    std::string describe(const ClipId& id) {
        std::string s = "{sprite=" + describe(id.sprite_id) +
                        ", action_type=" + std::to_string(id.action.index()) +
                        ", action=" + std::visit([](auto a) { return std::to_string(static_cast<long long>(a)); }, id.action) +
                        ", dir=" + describe(id.direction);
        if (id.item_id) s += ", item=" + describe(*id.item_id);
        return s + "}";
    }

    void report_missing(const char* what, const std::string& key) {
        static std::set<std::string> reported;
        const std::string msg = std::string(what) + " " + key;
        if (reported.insert(msg).second) {
            std::cerr << "[AssetManager] Missing def: " << msg << " -> using placeholder\n";
        }
    }

    template <class Map>
    const typename Map::mapped_type& lookup(const Map& m,
                                            const typename Map::key_type& key,
                                            const char* what,
                                            const typename Map::mapped_type& fallback) {
        const auto it = m.find(key);
        if (it != m.end()) return it->second;
        report_missing(what, describe(key));
        return fallback;
    }
}

AssetManager::AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) {
    for (const auto& t : TEXTURES) {
        const std::filesystem::path path = assets_root / "sprites" / t.path;
        SDL_Surface* raw = IMG_Load(path.string().c_str());
        if (!raw) throw std::runtime_error("Could not load: " + path.string() + ": " + IMG_GetError());
        textures.emplace(t.id, SDL2pp::Texture(renderer, SDL2pp::Surface(raw)));
    }

    if (Mix_QuerySpec(nullptr, nullptr, nullptr) == 0) return;
    for (const auto& def : SoundDefs::SOUNDS) {
        const std::string path = (assets_root / "sounds" / def.file).string();
        Mix_Chunk* raw = Mix_LoadWAV(path.c_str());
        if (!raw) {
            std::cerr << "[Audio] No se pudo cargar " << path << ": " << Mix_GetError() << '\n';
            continue;
        }
        sounds.insert_or_assign(def.id, Sound{std::shared_ptr<Mix_Chunk>(raw, Mix_FreeChunk), def.gain});
    }
}

SDL2pp::Texture& AssetManager::get_texture(SpriteId id) {
    auto it = textures.find(id);
    if (it == textures.end()) {
        report_missing("Texture SpriteId", describe(id));
        return textures.at(SpriteId::PlaceHolder);
    }
    return it->second;
}

const SpriteId& AssetManager::get_sprite_id(EnemyType type, bool frenzied) const {
    const EnemyVisual& v = lookup(ENEMY_VISUALS, type, "EnemyType", MISSING_ENEMY_VISUAL);
    return frenzied ? v.frenzy_sprite : v.sprite;
}

const int& AssetManager::get_height(EnemyType type) const {
    return lookup(ENEMY_VISUALS, type, "EnemyType", MISSING_ENEMY_VISUAL).cell_h;
}

const SpriteId& AssetManager::get_sprite_id(TrapType type) const {
    return lookup(TRAP_VISUALS, type, "TrapType", MISSING_TRAP_VISUAL).sprite;
}

const int& AssetManager::get_height(TrapType type) const {
    return lookup(TRAP_VISUALS, type, "TrapType", MISSING_TRAP_VISUAL).cell_h;
}

const int& AssetManager::get_depth_offset(TrapType type) const {
    return lookup(TRAP_VISUALS, type, "TrapType", MISSING_TRAP_VISUAL).depth_offset;
}

Direction AssetManager::get_clip_dir(TrapType type, Direction facing) const {
    const TrapVisual& v = lookup(TRAP_VISUALS, type, "TrapType", MISSING_TRAP_VISUAL);
    return v.directional ? facing : Direction::None;
}

const Clip& AssetManager::get_clip(const ClipId& id) const {
    return lookup(CLIPS, id, "Clip", placeholders::PLACEHOLDER_CLIP);
}

const Sound& AssetManager::get_sound(SoundId id) const {
    return lookup(sounds, id, "SoundId", placeholders::PLACEHOLDER_SOUND);
}

const Sprite& AssetManager::get_sprite(ItemId id) const {
    return lookup(ITEMS, id, "ItemId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(UiSpriteId id) const {
    return lookup(UI, id, "UiSpriteId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(InteriorId id) const {
    return lookup(INTERIORS, id, "InteriorId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(NpcId id) const {
    return lookup(NPCS, id, "NpcId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(EmoteId id) const {
    return lookup(EMOTES, id, "EmoteId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(Quality type) const {
    return lookup(QUALITIES, type, "Quality", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(PortalId id) const {
    return lookup(PORTALS, id, "PortalId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(TileTextureId id) const {
    return lookup(TILES, id, "TileTextureId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(DecorId id) const {
    return lookup(DECOR, id, "DecorId", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(BuildingId id) const {
    return lookup(BUILDINGS, id, "BuildingId", placeholders::PLACEHOLDER_SPRITE);
}

const Sprite& AssetManager::get_sprite(FramePieceId id) const {
    return lookup(FRAMES, id, "FramePieceId", placeholders::PLACEHOLDER_SPRITE);
}

const Sprite& AssetManager::get_tile_sprite(TileTextureId id, int tile_x, int tile_y) const {
    const auto it = TILE_PIECES.find(id);
    if (it == TILE_PIECES.end() || it->second.empty()) {
        report_missing("TileTextureId", describe(id));
        return placeholders::PLACEHOLDER_SPRITE;
    }
    const Sprite& whole = get_sprite(id);
    const int cols = std::max(1, whole.get_width() / TILE_TO_PIXEL);
    const int rows = std::max(1, whole.get_height() / TILE_TO_PIXEL);
    const int col = ((tile_x % cols) + cols) % cols;
    const int row = ((tile_y % rows) + rows) % rows;
    return it->second[static_cast<std::size_t>(row * cols + col)];
}

const Sprite& AssetManager::get_sprite(CropType type, CropState state) const {
    return lookup(CROPS, {type, state}, "Crop(type,state)", placeholders::PLACEHOLDER_SPRITE);
}
const Sprite& AssetManager::get_sprite(ResourceType type, ResourceState state) const {
    return lookup(ORES, {type, state}, "Resource(type,state)", placeholders::PLACEHOLDER_SPRITE);
}

const FlippableSprite& AssetManager::get_sprite(FurnitureId id, Direction dir) const {
    return lookup(FURNITURE, {id, dir}, "Furniture(id,dir)", placeholders::PLACEHOLDER_FLIPPABLESPRITE);
}

const SpriteId& AssetManager::get_sprite_id(AnimalType type) const {
    return lookup(ANIMAL_SPRITE_IDS, type, "AnimalType", placeholders::PLACEHOLDER_SPRITE_ID);
}

const ToolFrame& AssetManager::get_tool_frame(ItemId item, Direction dir, std::size_t frame_index) const {
    if (dir == Direction::Left) dir = Direction::Right;

    const auto it = TOOL_FRAMES.find({item, dir});
    if (it == TOOL_FRAMES.end() || it->second.empty()) {
        report_missing("ToolFrame(item,dir)", describe(std::pair{item, dir}));
        return placeholders::PLACEHOLDER_TOOLFRAME;
    }

    const auto& frames = it->second;
    return frames[std::min(frame_index, frames.size() - 1)];
}
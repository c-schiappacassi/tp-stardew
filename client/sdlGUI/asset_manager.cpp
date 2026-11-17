
#include "asset_manager.h"

#include <initializer_list>
#include <utility>
#include <string>
#include "dtos.h"
#include "sprite_defs.h"

namespace {
    SpriteDefs::ClipSetInfo with_sprite(SpriteDefs::ClipSetInfo set, SpriteId id) {
        set.sprite = id;
        return set;
    }

    template <class Map, class State, std::size_t N>
    void register_clips(Map& clips, const SpriteDefs::ClipSetInfo& set, const SpriteDefs::ClipDef<State> (&defs)[N], bool mirror_left, LayerOffsets (*offsets_of)(int) = nullptr) {
        for (const auto& d : defs) {
            Clip clip;
            clip.loop = d.loop;
            const bool flip = mirror_left && d.dir == Direction::Left;

            for (int i = 0; i < d.count; ++i) {
                const auto& f = d.frames[i];
                const int col = f.index % set.cols;
                const int row = f.index / set.cols;
                AnimatedFrame frame{
                    Sprite{set.sprite,
                        static_cast<uint16_t>(col * set.cell_w), static_cast<uint16_t>(row * set.cell_h),
                        set.cell_w, set.cell_h, set.off_x, set.off_y},
                    flip,
                    offsets_of ? offsets_of(f.index) : LayerOffsets{}
                };
                clip.sequence.push_back({frame, f.ms});
            }
            clips.insert_or_assign(ClipId{set.sprite, Action{d.state}, d.dir, d.item}, std::move(clip));
        }
    }

    Sprite make_tile_sprite(const SpriteDefs::SpriteRect& r) {
        return Sprite{r.sprite, r.x, r.y, r.w, r.h};
    }

    Sprite make_standing_sprite(const SpriteDefs::SpriteRect& r) {
        return Sprite{r.sprite, r.x, r.y, r.w, r.h, static_cast<int16_t>(-r.w / 2), static_cast<int16_t>(-r.h)};
    }

    template <class Map, class State, std::size_t N>
    void register_rect_clips(Map& clips, SpriteId sprite, const SpriteDefs::RectClipDef<State> (&defs)[N], bool mirror_right_to_left) {
        auto build = [&](const SpriteDefs::RectClipDef<State>& d, bool flip) {
            Clip clip;
            clip.loop = d.loop;
            for (int i = 0; i < d.count; ++i) {
                const auto& f = d.frames[i];
                AnimatedFrame frame{make_standing_sprite({sprite, f.x, f.y, f.w, f.h}), flip, LayerOffsets{}};
                clip.sequence.push_back({frame, f.ms});
            }
            return clip;
        };

        for (const auto& d : defs) {
            clips.insert_or_assign(ClipId{sprite, Action{d.state}, d.dir}, build(d, false));
            if (mirror_right_to_left && d.dir == Direction::Right) {
                clips.insert_or_assign(ClipId{sprite, Action{d.state}, Direction::Left}, build(d, true));
            }
        }
    }
}

SDL2pp::Texture AssetManager::load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path) {
    SDL_Surface* raw = IMG_Load(path.string().c_str());
    if (!raw) { throw std::runtime_error("Could not load: " + path.string() + ": " + IMG_GetError()); }
    return SDL2pp::Texture(renderer, SDL2pp::Surface(raw));
}

AssetManager::AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) {
    load_sprites_texture(renderer, assets_root / "sprites");
    load_animal_ids();
    load_farmer_clips();
    load_ore_sprites();
    load_item_sprites();
    load_ui_sprites();
    load_crop_sprites();
    load_cow_clips();
    load_chicken_clips();
    load_tool_frames();
    load_tile_sprites();
    load_decor_sprites();
    load_building_sprites();
    load_furniture_sprites();
    load_skeleton_clips();
    load_golem_clips();
    load_bat_clips();
    load_skull_clips();
    load_trap_clips();
    load_projectile_clips();
    load_portals();
    load_emotes();
    load_interiors();
    load_npcs();
}

void AssetManager::load_portals() {
    for (const auto& d : SpriteDefs::PORTALS)
        portal_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
}

const Sprite& AssetManager::get_sprite(PortalId id) const { return portal_sprites.at(id); }

void AssetManager::load_interiors() {
    for (const auto& d : SpriteDefs::INTERIORS)
        interior_sprites.insert_or_assign(d.id, make_tile_sprite(d.rect));
}

void AssetManager::load_npcs() {
    for (const auto& d : SpriteDefs::NPCS)
        npc_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
}

const Sprite& AssetManager::get_sprite(InteriorId id) const { return interior_sprites.at(id); }
const Sprite& AssetManager::get_sprite(NpcId id) const { return npc_sprites.at(id); }

void AssetManager::load_trap_clips() {
    register_clips(clips, SpriteDefs::SPIKE_SET, SpriteDefs::SPIKE_CLIPS, false);
    register_rect_clips(clips, SpriteId::WallFlameTrap, SpriteDefs::WALL_FLAME_CLIPS, true);
}

void AssetManager::load_projectile_clips() {
    register_clips(clips, SpriteDefs::FLAME_SET, SpriteDefs::FLAME_CLIPS, false);
}

void AssetManager::load_skull_clips() {
    register_clips(clips, SpriteDefs::SKULL_SET, SpriteDefs::SKULL_CLIPS, false);
}

void AssetManager::load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& sprites_root) {
    for (const auto& t : SpriteDefs::TEXTURES)
        textures.emplace(t.id, load_texture(renderer, sprites_root / t.path));
}

void AssetManager::load_ore_sprites() {
    for (const auto& d : SpriteDefs::ORES)
        ore_sprites.insert_or_assign(
            {d.type, d.state},
            Sprite{SpriteId::Ores, d.x, d.y, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE,
                   -Dimentions::ORE_SIZE / 2, -Dimentions::ORE_SIZE});
}

void AssetManager::load_crop_sprites() {
    for (const auto& d : SpriteDefs::CROPS)
        crop_sprites.insert_or_assign(
            {d.type, d.state},
            Sprite{SpriteId::Crops, d.x, d.y, Dimentions::CROP_CELL_W, Dimentions::CROP_CELL_H,
                   -Dimentions::CROP_CELL_W / 2, -Dimentions::CROP_CELL_H});
}

void AssetManager::load_item_sprites() {
    for (const auto& d : SpriteDefs::ITEMS)
        item_sprites.insert_or_assign(d.id, Sprite{d.sprite, d.x, d.y, d.w, d.h});
}

void AssetManager::load_emotes() {
    for (const auto& d : SpriteDefs::EMOTES)
        emote_sprites.insert_or_assign(d.id, Sprite{d.sprite, d.x, d.y, d.w, d.h,
            static_cast<int16_t>(-d.w / 2), static_cast<int16_t>(-d.h)});
}

void AssetManager::load_ui_sprites() {
    for (const auto& d : SpriteDefs::UI)
    ui_sprites.insert_or_assign(d.id, Sprite{d.sheet, d.x, d.y, d.w, d.h});
}

void AssetManager::load_tile_sprites() {
    for (const auto& d : SpriteDefs::TILES)
        tile_sprites.insert_or_assign(d.id, make_tile_sprite(d.rect));
}

void AssetManager::load_decor_sprites() {
    for (const auto& d : SpriteDefs::DECOR)
        decor_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
}

void AssetManager::load_building_sprites() {
    for (const auto& d : SpriteDefs::BUILDINGS)
        building_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
}

void AssetManager::load_furniture_sprites() {
    constexpr Direction ALL[] = {Direction::None, Direction::Up, Direction::Down, Direction::Left, Direction::Right};
    for (const auto& d : SpriteDefs::FURNITURE) {
        const Sprite s = make_standing_sprite(d.rect);
        if (d.dir == Direction::None) {
            for (const Direction dir : ALL) furniture_sprites.emplace(std::pair{d.id, dir}, s);
        } else {
            furniture_sprites.insert_or_assign({d.id, d.dir}, s);
        }
    }
}

void AssetManager::load_farmer_clips() {
    register_clips(clips, SpriteDefs::FARMER_SET, SpriteDefs::FARMER_CLIPS, SpriteDefs::FARMER_MIRROR_LEFT, SpriteDefs::farmer_offsets);
}

void AssetManager::load_animal_ids() {
    animal_sprite_ids = {
        {AnimalType::BrownCow,     SpriteId::BronwCow},
        {AnimalType::WhiteCow,     SpriteId::WhiteCow},
        {AnimalType::BrownChicken, SpriteId::BronwChicken},
        {AnimalType::WhiteChicken, SpriteId::WhiteChicken},
        {AnimalType::BlueChicken,  SpriteId::BlueChicken},
        {AnimalType::VoidChicken,  SpriteId::VoidChicken},
    };
}

void AssetManager::load_cow_clips() {
    for (SpriteId id : {SpriteId::BronwCow, SpriteId::WhiteCow})
        register_clips(clips, with_sprite(SpriteDefs::COW_SET, id), SpriteDefs::COW_CLIPS, true);
}

void AssetManager::load_chicken_clips() {
    for (SpriteId id : {SpriteId::BronwChicken, SpriteId::WhiteChicken,
                        SpriteId::BlueChicken,  SpriteId::VoidChicken})
        register_clips(clips, with_sprite(SpriteDefs::CHICKEN_SET, id), SpriteDefs::CHICKEN_CLIPS, true);
}


void AssetManager::load_tool_frames() {
    for (const auto& d : SpriteDefs::TOOL_ANIMS) {
        std::vector<ToolFrame> frames;
        for (const auto& s : d.steps) {
            const auto& r = d.sprites[s.sprite];
            const int16_t off_x = s.off.x - Dimentions::FARMER_CELL_W / 2;
            const int16_t off_y = s.off.y - Dimentions::FARMER_FEET_Y;
            frames.push_back(ToolFrame{
                Sprite{d.sheet, r.x, r.y, r.w, r.h, off_x, off_y}, 
                s.angle, SDL2pp::Point(d.pivot_x, d.pivot_y)});
        }
        tool_frames.insert_or_assign({d.item, d.dir}, std::move(frames));
    }
}

void AssetManager::load_bat_clips() {
    register_clips(clips, SpriteDefs::FROST_BAT_SET, SpriteDefs::BAT_CLIPS, false);
    register_clips(clips, SpriteDefs::FROST_BAT_DEBRIS_SET, SpriteDefs::BAT_DEAD_CLIPS, false);
    register_clips(clips, SpriteDefs::FROST_BAT_FRENZY_SET, SpriteDefs::BAT_CLIPS, false);
    register_clips(clips, SpriteDefs::LAVA_BAT_SET, SpriteDefs::BAT_CLIPS, false);
    register_clips(clips, SpriteDefs::LAVA_BAT_DEBRIS_SET, SpriteDefs::BAT_DEAD_CLIPS, false);
    register_clips(clips, SpriteDefs::LAVA_BAT_FRENZY_SET, SpriteDefs::BAT_CLIPS, false);
}


void AssetManager::load_skeleton_clips() {
    register_clips(clips, SpriteDefs::SKELETON_SET, SpriteDefs::SKELETON_CLIPS, false);
    register_clips(clips, SpriteDefs::SKELETON_BONES_SET, SpriteDefs::SKELETON_DEAD_CLIPS, false);
}

void AssetManager::load_golem_clips() {
    register_clips(clips, SpriteDefs::GOLEM_SET, SpriteDefs::GOLEM_CLIPS, false);
    register_clips(clips, SpriteDefs::GOLEM_ROCKS_SET, SpriteDefs::GOLEM_DEAD_CLIPS, false);
}

SDL2pp::Texture& AssetManager::get_texture(SpriteId id) { return textures.at(id); }

const Clip& AssetManager::get_clip(const ClipId& id) const { return clips.at(id); }

const Sprite& AssetManager::get_sprite(ItemId id) const { return item_sprites.at(id); }

const Sprite& AssetManager::get_sprite(UiSpriteId id) const { return ui_sprites.at(id); }

const Sprite& AssetManager::get_sprite(EmoteId id) const { return emote_sprites.at(id); }

const Sprite& AssetManager::get_sprite(CropType type, CropState state) const { return crop_sprites.at({type, state}); }

const Sound& AssetManager::get_sound(SoundId id) const { return sounds.at(id); }

const Sprite& AssetManager::get_sprite(ResourceType type, ResourceState state) const { return ore_sprites.at({type, state});}

const Sprite& AssetManager::get_sprite(TileTextureId id) const { return tile_sprites.at(id); }
const Sprite& AssetManager::get_sprite(DecorId id) const { return decor_sprites.at(id); }
const Sprite& AssetManager::get_sprite(BuildingId id) const { return building_sprites.at(id); }
const Sprite& AssetManager::get_sprite(FurnitureId id, Direction dir) const {  return furniture_sprites.at({id, dir});}

const SpriteId& AssetManager::get_sprite_id(AnimalType type) const {
    return animal_sprite_ids.at(type);
}

const Sprite* AssetManager::find_sprite(ItemId id) const {
    const auto it = item_sprites.find(id);
    return it == item_sprites.end() ? nullptr : &it->second;
}

const ToolFrame* AssetManager::find_tool_frame(ItemId item, Direction dir, std::size_t i) const {
    if (dir == Direction::Left) dir = Direction::Right;
    const auto it = tool_frames.find({item, dir});
    if (it == tool_frames.end() || it->second.empty()) return nullptr;
    return &it->second[std::min(i, it->second.size() - 1)];
}


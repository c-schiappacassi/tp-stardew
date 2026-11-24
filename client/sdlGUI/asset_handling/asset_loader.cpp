#include "asset_loader.h"

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>

#include <SDL_image.h>

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

    Sprite make_tile_anchored_sprite(const SpriteDefs::SpriteRect& r) {
        const int tiles_w = (r.w + TILE_TO_PIXEL - 1) / TILE_TO_PIXEL;
        const int tiles_h = (r.h + TILE_TO_PIXEL - 1) / TILE_TO_PIXEL;
        return Sprite{r.sprite, r.x, r.y, r.w, r.h,
                      static_cast<int16_t>((tiles_w * TILE_TO_PIXEL - r.w) / 2),
                      static_cast<int16_t>(tiles_h * TILE_TO_PIXEL - r.h)};
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

    SDL2pp::Texture load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path) {
        SDL_Surface* raw = IMG_Load(path.string().c_str());
        if (!raw) { throw std::runtime_error("Could not load: " + path.string() + ": " + IMG_GetError()); }
        return SDL2pp::Texture(renderer, SDL2pp::Surface(raw));
    }

    void load_sprites_texture(AssetTables& out, SDL2pp::Renderer& renderer, const std::filesystem::path& sprites_root) {
        for (const auto& t : SpriteDefs::TEXTURES)
            out.textures.emplace(t.id, load_texture(renderer, sprites_root / t.path));
    }

    void load_portals(AssetTables& out) {
        for (const auto& d : SpriteDefs::PORTALS)
            out.portal_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
    }

    void load_interiors(AssetTables& out) {
        for (const auto& d : SpriteDefs::INTERIORS)
            out.interior_sprites.insert_or_assign(d.id, make_tile_sprite(d.rect));
    }

    void load_npcs(AssetTables& out) {
        for (const auto& d : SpriteDefs::NPCS)
            out.npc_sprites.insert_or_assign(d.id, make_standing_sprite(d.rect));
    }

    void load_trap_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::SPIKE_SET, SpriteDefs::SPIKE_CLIPS, false);
        register_rect_clips(out.clips, SpriteId::WallFlameTrap, SpriteDefs::WALL_FLAME_CLIPS, true);
    }

    void load_projectile_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::FLAME_SET, SpriteDefs::FLAME_CLIPS, false);
    }

    void load_skull_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::SKULL_SET, SpriteDefs::SKULL_CLIPS, false);
    }

    void load_ore_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::ORES)
            out.ore_sprites.insert_or_assign(
                {d.type, d.state},
                Sprite{SpriteId::Ores, d.x, d.y, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE,
                       -Dimentions::ORE_SIZE / 2, -Dimentions::ORE_SIZE});
    }

    void load_qualitys(AssetTables& out) {
        for (const auto& d : SpriteDefs::QUALITYS)
            out.quality_sprites.insert_or_assign(
                d.type,
                Sprite{SpriteId::Cursors, d.x, d.y, Dimentions::QUALITY_SIZE, Dimentions::QUALITY_SIZE,
                       -Dimentions::QUALITY_SIZE / 2, -Dimentions::QUALITY_SIZE});
    }


    void load_crop_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::CROPS)
            out.crop_sprites.insert_or_assign(
                {d.type, d.state},
                Sprite{SpriteId::Crops, d.x, d.y, Dimentions::CROP_CELL_W, Dimentions::CROP_CELL_H,
                       -Dimentions::CROP_CELL_W / 2, -Dimentions::CROP_CELL_H});
    }

    void load_item_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::ITEMS)
            out.item_sprites.insert_or_assign(d.id, Sprite{d.sprite, d.x, d.y, d.w, d.h});
    }

    void load_emotes(AssetTables& out) {
        for (const auto& d : SpriteDefs::EMOTES)
            out.emote_sprites.insert_or_assign(d.id, Sprite{d.sprite, d.x, d.y, d.w, d.h,
                static_cast<int16_t>(-d.w / 2), static_cast<int16_t>(-d.h)});
    }

    void load_ui_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::UI)
        out.ui_sprites.insert_or_assign(d.id, Sprite{d.sheet, d.x, d.y, d.w, d.h});
    }

    void load_tile_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::TILES) {
            out.tile_sprites.insert_or_assign(d.id, make_tile_sprite(d.rect));

            const auto& r = d.rect;
            std::vector<Sprite>& pieces = out.tile_pieces[d.id];
            pieces.clear();
            const int cols = std::max(1, r.w / TILE_TO_PIXEL);
            const int rows = std::max(1, r.h / TILE_TO_PIXEL);
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    pieces.push_back(Sprite{r.sprite,
                        static_cast<uint16_t>(r.x + col * TILE_TO_PIXEL),
                        static_cast<uint16_t>(r.y + row * TILE_TO_PIXEL),
                        static_cast<uint16_t>(std::min<int>(TILE_TO_PIXEL, r.w)),
                        static_cast<uint16_t>(std::min<int>(TILE_TO_PIXEL, r.h))});
                }
            }
        }
    }

    void load_decor_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::DECOR)
            out.decor_sprites.insert_or_assign(d.id, make_tile_anchored_sprite(d.rect));
    }

    void load_building_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::BUILDINGS)
            out.building_sprites.insert_or_assign(d.id, make_tile_anchored_sprite(d.rect));
    }

    void load_frame_sprites(AssetTables& out) {
        for (const auto& d : SpriteDefs::FRAMES)
            out.frame_sprites.insert_or_assign(d.id, make_tile_anchored_sprite(d.rect));
    }

    void load_chest_clips(AssetTables& out) {
        register_rect_clips(out.clips, SpriteId::Craftables, SpriteDefs::CHEST_CLIPS, false);
    }

    void load_furniture_sprites(AssetTables& out) {
        constexpr Direction ALL[] = {Direction::None, Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        for (const auto& d : SpriteDefs::FURNITURE) {
            const Sprite s = make_standing_sprite(d.rect);
            if (d.dir == Direction::None) {
                for (const Direction dir : ALL) out.furniture_sprites.emplace(std::pair{d.id, dir}, FlippableSprite{s, false});
            } else {
                out.furniture_sprites.insert_or_assign({d.id, d.dir}, FlippableSprite{s, false});
                if (SpriteDefs::FURNITURE_MIRROR_LEFT && d.dir == Direction::Right) {
                    out.furniture_sprites.emplace(std::pair{d.id, Direction::Left}, FlippableSprite{s, true});
                }
            }
        }
    }

    void load_farmer_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::FARMER_SET, SpriteDefs::FARMER_CLIPS, SpriteDefs::FARMER_MIRROR_LEFT, SpriteDefs::farmer_offsets);
    }

    void load_animal_ids(AssetTables& out) {
        out.animal_sprite_ids = {
            {AnimalType::BrownCow,     SpriteId::BronwCow},
            {AnimalType::WhiteCow,     SpriteId::WhiteCow},
            {AnimalType::BrownChicken, SpriteId::BronwChicken},
            {AnimalType::WhiteChicken, SpriteId::WhiteChicken},
            {AnimalType::BlueChicken,  SpriteId::BlueChicken},
            {AnimalType::VoidChicken,  SpriteId::VoidChicken},
        };
    }

    void load_cow_clips(AssetTables& out) {
        for (SpriteId id : {SpriteId::BronwCow, SpriteId::WhiteCow})
            register_clips(out.clips, with_sprite(SpriteDefs::COW_SET, id), SpriteDefs::COW_CLIPS, true);
    }

    void load_chicken_clips(AssetTables& out) {
        for (SpriteId id : {SpriteId::BronwChicken, SpriteId::WhiteChicken,
                            SpriteId::BlueChicken,  SpriteId::VoidChicken})
            register_clips(out.clips, with_sprite(SpriteDefs::CHICKEN_SET, id), SpriteDefs::CHICKEN_CLIPS, true);
    }

    void load_tool_frames(AssetTables& out) {
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
            out.tool_frames.insert_or_assign({d.item, d.dir}, std::move(frames));
        }
    }

    void load_bat_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::FROST_BAT_SET, SpriteDefs::BAT_CLIPS, false);
        register_clips(out.clips, SpriteDefs::FROST_BAT_DEBRIS_SET, SpriteDefs::BAT_DEAD_CLIPS, false);
        register_clips(out.clips, SpriteDefs::FROST_BAT_FRENZY_SET, SpriteDefs::BAT_CLIPS, false);
        register_clips(out.clips, SpriteDefs::LAVA_BAT_SET, SpriteDefs::BAT_CLIPS, false);
        register_clips(out.clips, SpriteDefs::LAVA_BAT_DEBRIS_SET, SpriteDefs::BAT_DEAD_CLIPS, false);
        register_clips(out.clips, SpriteDefs::LAVA_BAT_FRENZY_SET, SpriteDefs::BAT_CLIPS, false);
    }


    void load_skeleton_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::SKELETON_SET, SpriteDefs::SKELETON_CLIPS, false);
        register_clips(out.clips, SpriteDefs::SKELETON_BONES_SET, SpriteDefs::SKELETON_DEAD_CLIPS, false);
    }

    void load_golem_clips(AssetTables& out) {
        register_clips(out.clips, SpriteDefs::GOLEM_SET, SpriteDefs::GOLEM_CLIPS, false);
        register_clips(out.clips, SpriteDefs::GOLEM_ROCKS_SET, SpriteDefs::GOLEM_DEAD_CLIPS, false);
    }

    void load_enemy_visuals(AssetTables& out) {
        auto& e = out.enemy_visuals;
        e[EnemyType::Skeleton]      = {SpriteId::Skeleton,   SpriteId::Skeleton,        Dimentions::SKELETON_CELL_H};
        e[EnemyType::Orc]           = {SpriteId::StoneGolem, SpriteId::StoneGolem,      Dimentions::GOLEM_CELL_H};
        e[EnemyType::VampireMale]   = {SpriteId::FrostBat,   SpriteId::FrostBatFrenzy,  Dimentions::BAT_CELL_H};
        e[EnemyType::VampireFemale] = {SpriteId::LavaBat,    SpriteId::LavaBatFrenzy,   Dimentions::BAT_CELL_H};
        e[EnemyType::Skull]         = {SpriteId::Skull,      SpriteId::Skull,           Dimentions::SKULL_CELL_H};
    }
    void load_trap_visuals(AssetTables& out) {
        auto& t = out.trap_visuals;
        t[TrapType::Spikes]    = {SpriteId::SpikeTrap,     Dimentions::SPIKE_CELL_H,     -TILE_TO_PIXEL, false};
        t[TrapType::WallFlame] = {SpriteId::WallFlameTrap, Dimentions::WALL_FLAME_MAX_H, 0,              true};
    }
}

AssetTables load_assets(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) {
    AssetTables out;
    load_sprites_texture(out, renderer, assets_root / "sprites");
    load_animal_ids(out);
    load_farmer_clips(out);
    load_ore_sprites(out);
    load_item_sprites(out);
    load_ui_sprites(out);
    load_crop_sprites(out);
    load_cow_clips(out);
    load_chicken_clips(out);
    load_tool_frames(out);
    load_tile_sprites(out);
    load_decor_sprites(out);
    load_building_sprites(out);
    load_frame_sprites(out);
    load_furniture_sprites(out);
    load_chest_clips(out);
    load_skeleton_clips(out);
    load_golem_clips(out);
    load_bat_clips(out);
    load_skull_clips(out);
    load_trap_clips(out);
    load_projectile_clips(out);
    load_portals(out);
    load_emotes(out);
    load_interiors(out);
    load_npcs(out);
    load_qualitys(out);
    load_enemy_visuals(out);
    load_trap_visuals(out);
    return out;
}

#pragma once

#include <algorithm>
#include <optional>

#include "../../utils.h"
#include "../../asset_handling/sprite_defs.h"
#include "../../draw_helpers.h"
#include "../entity_view.h"
#include "protocol_types.h"
#include "dtos.h"
#include "common/map/map_shapes.h"

class TileView {
private:
    Position pos;
    const Sprite& texture_sprite;
    const Sprite* under = nullptr;
public:
    explicit TileView(const TileData& data, AssetManager& assets)
        : pos(data.pos),
          texture_sprite(assets.get_tile_sprite(data.id, data.pos.getX() / TILE_TO_PIXEL, data.pos.getY() / TILE_TO_PIXEL)) {
        if (SpriteDefs::is_over_dungeon_floor(data.id)) {
            under = &assets.get_tile_sprite(TileTextureId::DungeonFloor, 0, 0);
        }
    }
    void draw(const RenderInfo& ctx){
        if (under) under->draw_at(ctx, pos);
        texture_sprite.draw_at(ctx, pos);
    };
};


class StaticSpriteView : public EntityView {
private:
    Position pos;
    const Sprite& sprite;
    bool flip;
    std::optional<int> sort_y;
public:
    StaticSpriteView(Position pos, const Sprite& sprite, bool flip = false, std::optional<int> sort_y = std::nullopt)
        : pos(pos), sprite(sprite), flip(flip), sort_y(sort_y) {}

    void draw(const RenderInfo& ctx) override {
        sprite.draw_at(ctx, pos, {.flip = flip});
    }

    std::optional<float> depth(const RenderInfo& ctx) const override {
        const int bottom = sprite.get_dy() + sprite.get_height();
        const Position center(pos.getX() + sprite.get_dx() + sprite.get_width() / 2, pos.getY() + sprite.get_dy() + sprite.get_height() / 2);
        if (!on_screen(ctx, center, std::max(sprite.get_width(), sprite.get_height()))) {
            return std::nullopt;
        }
        return static_cast<float>(sort_y ? *sort_y : static_cast<int>(pos.getY()) + bottom);
    }
};

// TODO:
class Background {
private:
    std::vector<TileView> floor;
    std::vector<StaticSpriteView> flat_props;  // portales planos en el piso
    std::vector<StaticSpriteView> props;       // decor + buildings + furniture + portales parados

    struct InteriorSprite {
        Position pos;
        const Sprite& sprite;
    };
    std::optional<InteriorSprite> interior;
    SDL2pp::Point world_size{0, 0};

    static bool is_flat(PortalId id) {
        return id == PortalId::ToNextLevel;
    }
    static bool is_flat(FurnitureId id) {
        return id == FurnitureId::Bed || id == FurnitureId::BeigeRug || id == FurnitureId::BlueRug;
    }
public:
    explicit Background(const BackgroundData& data, AssetManager& assets) {
        if (data.interior) {
            interior.emplace(InteriorSprite{data.interior->pos, assets.get_sprite(data.interior->interior_id)});
            world_size.x = interior->pos.getX() + interior->sprite.get_width();
            world_size.y = interior->pos.getY() + interior->sprite.get_height();
        }

        floor.reserve(data.floor.size());
        for (const auto& t : data.floor) {
            floor.emplace_back(t, assets);
            world_size.x = std::max<int>(world_size.x, t.pos.getX() + TILE_TO_PIXEL);
            world_size.y = std::max<int>(world_size.y, t.pos.getY() + TILE_TO_PIXEL);
        }

        props.reserve(data.decor.size() + data.frame.size() + data.buildings.size() + data.furniture.size() + data.portals.size() + 1);
        for (const DecorData& d : data.decor)
            props.emplace_back(d.pos, assets.get_sprite(d.id));
        for (const FramePieceData& f : data.frame)
            props.emplace_back(f.pos, assets.get_sprite(f.id));
        for (const BuildingData& b : data.buildings)
            props.emplace_back(b.pos, assets.get_sprite(b.id), false, map_rules::doorRowSortY(b.id, b.pos.getY(), TILE_TO_PIXEL));
        for (const StaticFurnitureData& f : data.furniture) {
            const FlippableSprite& fs = assets.get_sprite(f.id, f.dir);
            if (is_flat(f.id)) {
                flat_props.emplace_back(f.pos, fs.get_sprite(), fs.is_flipped());
            } else if (f.id == FurnitureId::BedSheet) {
                props.emplace_back(f.pos, fs.get_sprite(), fs.is_flipped(), static_cast<int>(f.pos.getY()) + 1);
            } else {
                props.emplace_back(f.pos, fs.get_sprite(), fs.is_flipped());
            }
        }
        if (data.interior && data.interior->npc) {
            const NpcData& npc = *data.interior->npc;
            props.emplace_back(npc.pos, assets.get_sprite(npc.id));
        }

        for (const PortalData& p : data.portals) {
            auto& target = is_flat(p.id) ? flat_props : props;
            const int sort_y = map_rules::portalSortY(p.id, p.pos.getY(), TILE_TO_PIXEL);
            target.emplace_back(p.pos, assets.get_sprite(p.id), false, sort_y);
        }
    }

    SDL2pp::Point size() const { return world_size; }

    void draw(const RenderInfo& ctx) {
        if (interior) {
            interior->sprite.draw_at(ctx, interior->pos);
        }
        for (TileView& t : floor)
            t.draw(ctx);
        for (StaticSpriteView& p : flat_props) {
            if (p.depth(ctx)) p.draw(ctx);
        }
    }

    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) {
        for (StaticSpriteView& p : props) {
            if (const auto depth = p.depth(ctx)) {
                out.push_back(Drawable{*depth, tie++, &p});
            }
        }
    }
};

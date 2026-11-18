#pragma once

#include "../utils.h"
#include "../draw_region.h"
#include "entity_view.h"
#include "protocol_types.h"
#include "dtos.h"

class TileView {
private:
    Position pos;
    const Sprite& texture_sprite;
public:
    explicit TileView(const TileData& data, AssetManager& assets): pos(data.pos), texture_sprite(assets.get_sprite(data.id)){};
    void draw(const RenderInfo& ctx){
        draw_sprite(ctx, this->texture_sprite, ctx.cam.world_to_screen(pos.getX(), pos.getY()));
    };
};


class StaticSpriteView : public EntityView {
private:
    Position pos;
    const Sprite& sprite;
    bool flip;
public:
    StaticSpriteView(Position pos, const Sprite& sprite, bool flip = false)
        : pos(pos), sprite(sprite), flip(flip) {}

    void draw(const RenderInfo& ctx) override {
        draw_sprite(ctx, sprite, ctx.cam.world_to_screen(pos.getX(), pos.getY()), flip);
    }

    std::optional<float> depth(const RenderInfo& ctx) const override {
        const int radius = sprite.height * ctx.cam.get_zoom();
        if (!ctx.cam.is_visible(pos.getX(), pos.getY(), radius)) {
            return std::nullopt;
        }
        return static_cast<float>(pos.getY());
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

    static bool is_flat(PortalId id) {
        return id == PortalId::ToNextLevel;
    }

public:
    explicit Background(const BackgroundData& data, AssetManager& assets) {
        if (data.interior) {
            interior.emplace(InteriorSprite{data.interior->pos, assets.get_sprite(data.interior->interior_id)});
        }

        floor.reserve(data.floor.size());
        for (const auto& t : data.floor)
            floor.emplace_back(t, assets);

        props.reserve(data.decor.size() + data.buildings.size() + data.furniture.size() + data.portals.size() + 1);
        for (const DecorData& d : data.decor)
            props.emplace_back(d.pos, assets.get_sprite(d.id));
        for (const BuildingData& b : data.buildings)
            props.emplace_back(b.pos, assets.get_sprite(b.id));
        for (const StaticFurnitureData& f : data.furniture) {
            const FlippableSprite& fs = assets.get_sprite(f.id, f.dir);
            props.emplace_back(f.pos, fs.sprite, fs.flip);
        }
        if (data.interior && data.interior->npc) {
            const NpcData& npc = *data.interior->npc;
            props.emplace_back(npc.pos, assets.get_sprite(npc.id));
        }

        for (const PortalData& p : data.portals) {
            auto& target = is_flat(p.id) ? flat_props : props;
            target.emplace_back(p.pos, assets.get_sprite(p.id));
        }
    }

    void draw(const RenderInfo& ctx) {
        if (interior) {
            draw_sprite(ctx, interior->sprite, ctx.cam.world_to_screen(interior->pos.getX(), interior->pos.getY()));
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
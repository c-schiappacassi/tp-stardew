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
    std::vector<StaticSpriteView> props; // decor + buildings

public:
    explicit Background(const BackgroundData& data, AssetManager& assets) {
        floor.reserve(data.floor.size());
        for (const auto& t : data.floor)
            floor.emplace_back(t, assets);

        props.reserve(data.decor.size() + data.buildings.size());
        for (const DecorData& d : data.decor)
            props.emplace_back(d.pos, assets.get_sprite(d.id));
        for (const BuildingData& b : data.buildings)
            props.emplace_back(b.pos, assets.get_sprite(b.id));
        for (const StaticFurnitureData& f : data.furniture)
            props.emplace_back(f.pos, assets.get_sprite(f.id, f.dir));
    }

    void draw(const RenderInfo& ctx){
        for (TileView& t : floor){
            t.draw(ctx);
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
#pragma once

#include <cstdint>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../background_zone_view.h"
#include "../furniture_view.h"
#include "../../sync_views.h"
#include "../../draw_region.h"
#include "crop_view.h"
#include "animal_view.h"

class FarmView : public BackgroundZoneView {
private:
    struct SoilTile { int x, y; bool watered; };

    std::unordered_map<uint32_t, CropView> crops;
    std::unordered_map<uint32_t, AnimalView> animals;
    std::vector<SoilTile> soil;
    std::unordered_map<uint32_t, FurnitureView> furniture;

    static std::pair<int, int> cell_of(const Position& base) {
        return {base.getX() / TILE_TO_PIXEL, (base.getY() - 1) / TILE_TO_PIXEL};
    }

protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) override;
    void draw_floor_overlay(const RenderInfo& ctx) override;

public:
    FarmView(const BackgroundData& data, AssetManager& assets);
    void update(const FarmData& data);
};

inline FarmView::FarmView(const BackgroundData& data, AssetManager& assets)
    : BackgroundZoneView(data, assets) {}

inline void FarmView::update(const FarmData& farm) {
    auto id_of = [](const auto& d) { return d.id; };
    sync_views(crops, farm.crops, id_of);
    sync_views(animals, farm.animals, id_of);
    sync_views(furniture, farm.furniture, id_of);

    std::set<std::pair<int, int>> watered;
    for (const auto& crop : farm.crops) {
        if (crop.wattered) watered.insert(cell_of(crop.pos));
    }

    soil.clear();
    soil.reserve(farm.tilledSoil.size());
    for (const auto& base : farm.tilledSoil) {
        const auto cell = cell_of(base);
        soil.push_back({cell.first * TILE_TO_PIXEL, cell.second * TILE_TO_PIXEL, watered.count(cell) > 0});
    }
}

inline void FarmView::draw_floor_overlay(const RenderInfo& ctx) {
    const Sprite& dry = ctx.assets.get_sprite(TileTextureId::TilledDirt);
    const Sprite& wet = ctx.assets.get_sprite(TileTextureId::WateredDirt);
    const int radius = TILE_TO_PIXEL * ctx.cam.get_zoom();

    for (const SoilTile& t : soil) {
        if (!ctx.cam.is_visible(t.x, t.y, radius)) continue;
        const SDL2pp::Point origin = ctx.cam.world_to_screen(t.x, t.y);
        draw_sprite(ctx, dry, origin);
        if (t.watered) draw_sprite(ctx, wet, origin);
    }
}

inline void FarmView::collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) {
    collect_from(ctx, crops, out, tie);
    collect_from(ctx, animals, out, tie);
    collect_from(ctx, furniture, out, tie);
}
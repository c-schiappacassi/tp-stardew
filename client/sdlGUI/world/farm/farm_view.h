#pragma once

#include <cstdint>

#include "../zone_view.h"
#include "../../sync_views.h"
#include "crop_view.h"
#include "animal_view.h"

class FarmView : public ZoneView {
private:
    std::unordered_map<uint32_t, CropView> crops;
    std::unordered_map<uint32_t, AnimalView> animals;
    // std::unordered_map<uint32_t, FurnitureView> Furniture; TODO: TBD si los muebles pueden ir donde sea en la granja o solo adentro de la casa
public:
    void update(const FarmData& data);
    void draw_background(const RenderInfo& ctx) override;
    void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) override;
};

void FarmView::update(const FarmData& farm) {
    auto id_of = [](const auto& d) { return d.id; };
    sync_views(crops, farm.crops, id_of);
    sync_views(animals, farm.animals, id_of);
}

void FarmView::draw_background(const RenderInfo&) {
    // TODO: implementar background
}

void FarmView::collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) {
    uint64_t tie = static_cast<uint64_t>(1ull) << 32;
    collect_from(ctx, crops, out, tie);
    collect_from(ctx, animals, out, tie);
    // collect_from(ctx, decor, out, tie);
}
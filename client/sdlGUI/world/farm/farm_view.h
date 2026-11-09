#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../background_zone_view.h"
#include "../../sync_views.h"
#include "crop_view.h"
#include "animal_view.h"

class FarmView : public BackgroundZoneView {
private:
    std::unordered_map<uint32_t, CropView> crops;
    std::unordered_map<uint32_t, AnimalView> animals;
    // std::unordered_map<uint32_t, FurnitureView> furniture; TODO: TBD si los muebles pueden ir donde sea en la granja o solo adentro de la casa
    // std::unordered_map<uint32_t, ChestView> para animar que se abre, pero ver lo de adentro va a ser cosa de hud
    
protected:
    void collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) override;

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
}

inline void FarmView::collect_entities(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) {
    collect_from(ctx, crops, out, tie);
    collect_from(ctx, animals, out, tie);
}
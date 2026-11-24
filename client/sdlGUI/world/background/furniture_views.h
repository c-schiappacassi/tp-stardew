#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "chest_view.h"
#include "furniture_view.h"
#include "../../sync_views.h"
#include "../../utils.h"

#include "dtos.h"
#include "protocol_types.h"

class FurnitureViews {
private:
    std::unordered_map<uint32_t, FurnitureView> plain;
    std::unordered_map<uint32_t, ChestView> chests;

    std::vector<FurnitureData> plain_data;
    std::vector<FurnitureData> chest_data;

public:
    void update(const std::vector<FurnitureData>& data) {
        plain_data.clear();
        chest_data.clear();
        for (const FurnitureData& d: data) {
            (d.type == FurnitureId::Chest ? chest_data : plain_data).push_back(d);
        }

        const auto id_of = [](const FurnitureData& d) { return d.id; };
        sync_views(plain, plain_data, id_of);
        sync_views(chests, chest_data, id_of);
    }

    void collect(const RenderInfo& ctx, std::vector<Drawable>& out, uint64_t& tie) {
        collect_from(ctx, plain, out, tie);
        collect_from(ctx, chests, out, tie);
    }
};

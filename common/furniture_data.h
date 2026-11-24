#pragma once

#include <algorithm>

#include "dtos.h"

inline const std::vector<FurnitureData>* zoneFurniture(const ZoneData& zone) {
    if (const auto* farm = std::get_if<FarmData>(&zone))
        return &farm->furniture;
    if (const auto* mine = std::get_if<MineData>(&zone))
        return &mine->current_level_data.furniture;
    if (const auto* inside = std::get_if<InteriorData>(&zone); inside && inside->interior_id == InteriorId::PlayerHouse)
        return &inside->furniture;
    return nullptr;
}

inline bool hasChest(const ZoneData& zone, uint32_t id) {
    const auto* furniture = zoneFurniture(zone);
    return furniture && id != 0 && std::any_of(furniture->begin(), furniture->end(), [id](const auto& item) { return item.id == id && item.type == FurnitureId::Chest; });
}

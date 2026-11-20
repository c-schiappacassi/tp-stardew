#include "farm_status.h"
#include "farm_manager.h"

#include <optional>

namespace {
std::optional<FurnitureId> toFurnitureId(ItemId item) {
    if (!Decoration::isDecoration(item)) return std::nullopt;
    return static_cast<FurnitureId>(static_cast<uint16_t>(item) - static_cast<uint16_t>(ItemId::Chair));
}
}  // namespace

FarmData FarmStatus::build(const FarmManager& farm) {
    FarmData result;
    for (const auto& [id, crop] : farm.getCrops()) {
        result.crops.push_back(crop.getPublicData());
    }
    for (const auto* animal : farm.getAnimals()) {
        result.animals.push_back(animal->getPublicData());
    }
    for (const auto& [id, decoration] : farm.getDecorations()) {
        const auto type = toFurnitureId(decoration.getItem());
        if (!type) continue;
        FurnitureData data;
        data.id = id;
        data.pos = decoration.getPosition();
        data.type = *type;
        data.dir = decoration.getDirection();
        result.furniture.push_back(data);
    }
    result.tilledSoil = farm.getTilledPositions();
    result.loot = farm.getGroundItems().getPublicData();
    return result;
}

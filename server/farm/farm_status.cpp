#include "farm_status.h"
#include "farm_manager.h"

#include <optional>

namespace {
std::optional<FurnitureId> toFurnitureId(ItemId item) {
    switch (item) {
        case ItemId::Chair: return FurnitureId::Chair;
        case ItemId::Table: return FurnitureId::Table;
        default:            return std::nullopt;
    }
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
        data.dir = Direction::Down;
        result.furniture.push_back(data);
    }
    result.tilledSoil = farm.getTilledPositions();
    return result;
}
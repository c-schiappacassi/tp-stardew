#include "crop_manager.h"
#include "farm/farm_config.h"

#include <utility>

#include "farm/spatial/farm_reservation.h"

CropManager::CropManager(const FarmConfig& config):
    definitions(config.crops), dayLength(config.dayLength) {
    config.validate();
}

bool CropManager::hasTilledSoil() const { return !tilledSoil.empty(); }

const std::map<EntityId, Crop>& CropManager::getCrops() const { return crops; }
const std::set<ZoneMap::Cell>& CropManager::getTilledSoil() const { return tilledSoil; }

const CropDefinition* CropManager::findDefinition(CropType type) const {
    for (const auto& definition : definitions) {
        if (definition.type == type) {
            return &definition;
        }
    }
    return nullptr;
}

bool CropManager::tillSoil(const Position& position, const FarmPlacement& placement) {
    const auto cell = placement.cellAt(position);
    if (!cell || !placement.isAvailable(CollisionBox{entity_dimensions::CROP}.at(position))) {
        return false;
    }
    return tilledSoil.insert(*cell).second;
}

bool CropManager::isTilled(const Position& position, const FarmPlacement& placement) const {
    const auto cell = placement.cellAt(position);
    return cell && tilledSoil.count(*cell) > 0;
}

bool CropManager::plant(EntityId id, const Position& position, CropType type,
    FarmPlacement& placement) {
    const auto* definition = findDefinition(type);
    if (!definition || id == 0 || placement.contains(id) || !isTilled(position, placement) ||
        !placement.isAvailable(CollisionBox{entity_dimensions::CROP}.at(position))) {
        return false;
    }
    Crop crop{id, position, *definition, dayLength};
    FarmReservation reservation{placement, id, position, CollisionBox{entity_dimensions::CROP}};
    if (!reservation.isReserved() || !crops.emplace(id, std::move(crop)).second) {
        return false;
    }
    reservation.commit();
    return true;
}

bool CropManager::water(EntityId id) {
    const auto it = crops.find(id);
    if (it == crops.end()) {
        return false;
    }
    return it->second.water();
}

std::optional<FarmProduct> CropManager::harvest(EntityId id, FarmPlacement& placement,
    double luck) {
    const auto it = crops.find(id);
    if (it == crops.end()) {
        return std::nullopt;
    }
    auto product = it->second.harvest(luck);
    if (it->second.isHarvested()) {
        placement.release(id);
        crops.erase(it);
    }
    return product;
}

void CropManager::update(std::chrono::milliseconds elapsed) {
    for (auto& [id, crop] : crops) {
        crop.update(elapsed);
    }
}

void CropManager::finishDay() {
    for (auto& [id, crop] : crops) crop.finishDay();
}

const Crop* CropManager::find(EntityId id) const {
    const auto it = crops.find(id);
    if (it == crops.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

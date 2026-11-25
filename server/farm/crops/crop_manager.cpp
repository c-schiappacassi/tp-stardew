#include "crop_manager.h"

#include "server_constants.h"
#include "farm/farm_config.h"

#include <stdexcept>
#include <utility>

#include "entities/placement/placement_reservation.h"
#include "farm/spatial/farm_terrain.h"

CropManager::CropManager(const FarmConfig& config):
    definitions(config.crops), dayLength(config.dayLength) {
    config.validate();
}

bool CropManager::hasTilledSoil() const { return !tilledSoil.empty(); }

const std::map<EntityId, Crop>& CropManager::getCrops() const { return crops; }
const std::set<ZoneMap::Cell>& CropManager::getTilledSoil() const { return tilledSoil; }

const std::map<ZoneMap::Cell, std::chrono::milliseconds>& CropManager::getEmptySoilElapsed() const {
    return emptySoilElapsed;
}

bool CropManager::restoreEmptySoilElapsed(const Position& position,
    std::chrono::milliseconds elapsed, const Placement& placement) {
    const auto cell = placement.cellAt(position);
    if (!cell || elapsed.count() < 0 || elapsed >= 2 * dayLength) return false;
    const auto it = emptySoilElapsed.find(*cell);
    if (it == emptySoilElapsed.end()) return false;
    it->second = elapsed;
    return true;
}

const CropDefinition* CropManager::findDefinition(CropType type) const {
    for (const auto& definition : definitions) {
        if (definition.type == type) {
            return &definition;
        }
    }
    return nullptr;
}

bool CropManager::tillSoil(const Position& position, const FarmTerrain& terrain) {
    const auto& placement = terrain.getSpace();
    const auto cell = placement.cellAt(position);
    if (!cell || !terrain.isCultivable(position) || !placement.isAvailable(CollisionBox{entity_dimensions::CROP}.at(position))) {
        return false;
    }
    if (!tilledSoil.insert(*cell).second) return false;
    emptySoilElapsed.emplace(*cell, std::chrono::milliseconds{0});
    return true;
}

bool CropManager::isTilled(const Position& position, const Placement& placement) const {
    const auto cell = placement.cellAt(position);
    return cell && tilledSoil.count(*cell) > 0;
}

bool CropManager::plant(EntityId id, const Position& position, CropType type,
    Placement& placement) {
    const auto* definition = findDefinition(type);
    const auto cell = placement.cellAt(position);
    if (!definition || id == 0 || placement.contains(id) || !cell || !tilledSoil.contains(*cell) ||
        !placement.isAvailable(CollisionBox{entity_dimensions::CROP}.at(position))) {
        return false;
    }
    Crop crop{id, position, *definition, dayLength};
    PlacementReservation reservation{placement, id, position, CollisionBox{entity_dimensions::CROP}};
    if (!reservation.isReserved() || !crops.emplace(id, std::move(crop)).second) {
        return false;
    }
    reservation.commit();
    emptySoilElapsed.erase(*cell);
    return true;
}

bool CropManager::water(EntityId id) {
    const auto it = crops.find(id);
    if (it == crops.end()) {
        return false;
    }
    return it->second.water();
}

std::optional<FarmProduct> CropManager::harvest(EntityId id, Placement& placement,
    double luck) {
    const auto it = crops.find(id);
    if (it == crops.end()) {
        return std::nullopt;
    }
    auto product = it->second.harvest(luck);
    if (it->second.isHarvested()) {
        const auto cell = placement.cellAt(it->second.getPosition());
        if (cell) emptySoilElapsed[*cell] = std::chrono::milliseconds{0};
        placement.release(id);
        crops.erase(it);
    }
    return product;
}

void CropManager::update(std::chrono::milliseconds elapsed) {
    if (elapsed.count() < 0) throw std::invalid_argument(server_constants::NEGATIVE_TIME);
    for (auto it = emptySoilElapsed.begin(); it != emptySoilElapsed.end();) {
        if (elapsed >= 2 * dayLength - it->second) {
            tilledSoil.erase(it->first);
            it = emptySoilElapsed.erase(it);
        } else {
            it->second += elapsed;
            ++it;
        }
    }
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

Crop* CropManager::findMutable(EntityId id) {
    const auto it = crops.find(id);
    return it == crops.end() ? nullptr : &it->second;
}

std::size_t CropManager::care() {
    std::size_t count = 0;
    for (auto& [id, crop]: crops) {
        if (crop.isHarvested())
            continue;
        crop.growFully();
        ++count;
    }
    return count;
}

std::vector<Position> CropManager::tilledPositions(const ZoneMap& map) const {
    std::vector<Position> result;
    for (const auto& cell: tilledSoil) {
        if (const auto base = map.baseAt(cell))
            result.push_back(*base);
    }
    return result;
}

std::vector<std::pair<Position, uint64_t>> CropManager::emptySoilElapsedData(const ZoneMap& map) const {
    std::vector<std::pair<Position, uint64_t>> result;
    for (const auto& [cell, elapsed]: emptySoilElapsed) {
        if (const auto base = map.baseAt(cell))
            result.emplace_back(*base, static_cast<uint64_t>(elapsed.count()));
    }
    return result;
}

std::optional<CropData> CropManager::dataFor(EntityId id) const {
    const auto* entity = find(id);
    return entity ? std::optional<CropData>{entity->getPublicData()} : std::nullopt;
}

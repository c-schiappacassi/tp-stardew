#include "world_persistence.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "persistence/persistence_dto.h"
#include "world/interiors/interior_rooms.h"

WorldPersistence::WorldPersistence(DungeonManager& dungeon, FarmManager& farm, InteriorRooms& interiors, EntityIdGenerator& ids):
    dungeon(dungeon), farm(farm), interiors(interiors), entityIds(ids) {}

void WorldPersistence::restoreFarm(const PersistenceDTO& saved) {
    farm.restoreData(FarmRestoreData{saved.farm, saved.cropGrowthProgress, saved.cropWaterRemainingTime, saved.harvestedCrops, saved.regrowingCrops,
        saved.animalProductionProgress, saved.animalAvailableProducts, saved.emptySoilElapsed, saved.chestInventories});
    uint32_t maximum = 0;
    for (const auto& item: saved.farm.crops) maximum = std::max(maximum, item.id);
    for (const auto& item: saved.farm.animals) maximum = std::max(maximum, item.id);
    for (const auto& item: saved.farm.furniture) maximum = std::max(maximum, item.id);
    for (const auto& item: saved.farm.loot) maximum = std::max(maximum, item.id);
    interiors.restoreStorage(saved.interiors, saved.chestInventories);
    dungeon.restoreStorage(saved.dungeonFurniture, saved.chestInventories, saved.dungeonStorageLoot);
    for (const auto& room: saved.interiors) {
        for (const auto& item: room.furniture) maximum = std::max(maximum, item.id);
        for (const auto& item: room.loot) maximum = std::max(maximum, item.id);
    }
    for (const auto& [level, furniture]: saved.dungeonFurniture) {
        for (const auto& item: furniture) maximum = std::max(maximum, item.id);
    }
    for (const auto& [level, loot]: saved.dungeonStorageLoot) {
        for (const auto& item: loot) maximum = std::max(maximum, item.id);
    }
    if (maximum == UINT32_MAX)
        throw std::runtime_error("Saved entity ID exhausted");
    entityIds.ensureNextId(maximum + 1);
}

void WorldPersistence::saveFarmAndStorage(PersistenceDTO& saved) const {
    auto farmData = farm.getPersistenceData();
    saved.farm = std::move(farmData.farm);
    saved.cropGrowthProgress = std::move(farmData.cropGrowthProgress);
    saved.cropWaterRemainingTime = std::move(farmData.cropWaterRemainingTime);
    saved.harvestedCrops = std::move(farmData.harvestedCrops);
    saved.regrowingCrops = std::move(farmData.regrowingCrops);
    saved.animalProductionProgress = std::move(farmData.animalProductionProgress);
    saved.animalAvailableProducts = std::move(farmData.animalAvailableProducts);
    saved.emptySoilElapsed = std::move(farmData.emptySoilElapsed);
    saved.chestInventories = std::move(farmData.chestInventories);
    saved.interiors = interiors.storageData();
    saved.dungeonFurniture = dungeon.storageFurniture();
    saved.dungeonStorageLoot = dungeon.storageLoot();
    interiors.saveInventories(saved.chestInventories);
    dungeon.saveInventories(saved.chestInventories);
}

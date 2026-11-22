#include <utility>

#include "player/player_manager.h"

#include "world_manager.h"

void WorldManager::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    switch (players.snapshot(playerId).getZone()) {
        case Zone::Farm:
            farm.depositInChest(players, playerId, command);
            break;
        case Zone::Dungeon:
            dungeon.depositInChest(players, playerId, command);
            break;
        case Zone::InsideHouse:
            interiors.depositInChest(players, playerId, command);
            break;
        default:
            break;
    }
}

void WorldManager::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    switch (players.snapshot(playerId).getZone()) {
        case Zone::Farm:
            farm.withdrawFromChest(players, playerId, command);
            break;
        case Zone::Dungeon:
            dungeon.withdrawFromChest(players, playerId, command);
            break;
        case Zone::InsideHouse:
            interiors.withdrawFromChest(players, playerId, command);
            break;
        default:
            break;
    }
}

void WorldManager::saveFarmAndStorage(PersistenceDTO& saved) const {
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

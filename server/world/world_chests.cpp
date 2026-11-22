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

void WorldManager::saveChestStorage(PersistenceDTO& saved) const {
    for (const auto& [id, chest]: farm.getChests()) saved.chestInventories.emplace(id, chest.getInventory().getItems());
    saved.interiors = interiors.storageData();
    saved.dungeonFurniture = dungeon.storageFurniture();
    saved.dungeonStorageLoot = dungeon.storageLoot();
    interiors.saveInventories(saved.chestInventories);
    dungeon.saveInventories(saved.chestInventories);
}

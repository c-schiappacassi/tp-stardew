#include "player/player_manager.h"

#include "world_manager.h"

bool WorldManager::loadFarm(uint8_t worldId, const FarmLayout& layout) {
    if (loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    FarmManager preparedFarm{farm.getConfig(), farmSeed};
    auto preparedIds = entityIds;
    InteriorManager preparedInteriors;
    if (!preparedFarm.initialize(layout, preparedIds) || !preparedInteriors.initialize(layout.interiors)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (const auto& room: layout.interiors) {
        if (room.location.interior_id == InteriorId::PlayerHouse && room.location.house_index >= layout.playerHouses)
            return false;
        if (room.outside.zone == Zone::Farm && (!preparedFarm.canWalk(playerBox.at(room.outside.position)) || !preparedFarm.canWalk(playerBox.at(room.returnPosition))))
            return false;
    }
    farm = std::move(preparedFarm);
    interiors = std::move(preparedInteriors);
    entityIds = preparedIds;
    loadedWorld = worldId;
    travel.configureFarm(layout);
    town = TownMap{};
    return true;
}

const FarmManager& WorldManager::getFarm() const { return farm; }

void WorldManager::update(std::chrono::milliseconds elapsed, PlayerManager& players, std::chrono::steady_clock::time_point now) {
    if (farm.isInitialized()) {
        farm.setObstacles(players.getCollisionBounds(Zone::Farm));
        farm.update(elapsed, entityIds);
    }
    const auto playerIds = players.connectedPlayerIds();
    for (const auto id: playerIds) interiors.useBed(players, id);
    dungeon.update(elapsed, players.combatants(), players, entityIds, now);
    for (const auto id: playerIds) {
        const auto zone = players.snapshot(id).getZone();
        if (zone == Zone::Farm)
            farm.collectNearbyItems(players, id);
        else if (zone == Zone::Dungeon)
            dungeon.collectNearbyItems(players, id);
    }
}

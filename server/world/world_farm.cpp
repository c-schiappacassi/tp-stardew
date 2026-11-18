#include "world_manager.h"

#include "player/player.h"

bool WorldManager::loadFarm(uint8_t worldId, const FarmLayout& layout) {
    if (loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    FarmManager preparedFarm{farm.getConfig(), farmSeed};
    auto preparedIds = entityIds;
    InteriorManager preparedInteriors;
    if (!preparedFarm.initialize(layout, preparedIds) ||
        !preparedInteriors.initialize(layout.interiors)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (const auto& room: layout.interiors) {
        if (room.location.interior_id == InteriorId::PlayerHouse &&
            room.location.house_index >= layout.playerHouses) return false;
        if (room.outside.zone == Zone::Farm &&
            (!preparedFarm.canWalk(playerBox.at(room.outside.position)) ||
             !preparedFarm.canWalk(playerBox.at(room.returnPosition)))) return false;
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

void WorldManager::update(std::chrono::milliseconds elapsed,const std::vector<CollisionBounds>& farmPlayers,const std::vector<Combatant>& players,std::chrono::steady_clock::time_point now) {
    if (farm.isInitialized()) {
        farm.setObstacles(farmPlayers);
        farm.update(elapsed, entityIds);
    }
    for (const auto& combatant: players) {
        if (combatant.player) interiors.useBed(*combatant.player);
    }
    dungeon.update(elapsed, players, entityIds, now);
    
    for (const auto& actor: players) {
        if (!actor.player) continue;
        if (actor.player->getZone() == Zone::Farm) {
            farm.collectNearbyItems(*actor.player);
        } else if (actor.player->getZone() == Zone::Dungeon) {
            dungeon.collectNearbyItems(*actor.player);
        }
    }
}

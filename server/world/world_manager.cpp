#include "world_manager.h"
#include "player/player.h"

WorldManager::WorldManager(const DungeonConfig& config, uint32_t seed, const FarmConfig& farmConfig)
    : dungeon(config, seed), farm(farmConfig, seed) {}

bool WorldManager::spawnPlayer(Player& player) const {
    return farm.isInitialized() ? farm.spawnPlayer(player) : dungeon.spawnPlayer(player);
}

bool WorldManager::loadDungeon(uint8_t worldId,const std::map<uint8_t, DungeonRoomLayout>& layout) {
    if (farm.isInitialized() && loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    if (!dungeon.initialize(layout, entityIds)) {
        return false;
    }
    loadedWorld = worldId;
    entrance.reset();
    return true;
}

bool WorldManager::configureDungeonEntrance(const DungeonEntrance& data) {
    if (!loadedWorld || (data.zone != Zone::Farm && data.zone != Zone::Town) ||!data.terrain.isWalkableBase(data.position) || !data.terrain.isWalkableBase(data.returnPosition)) {
        return false;
    }
    if (farm.isInitialized() && data.zone == Zone::Farm &&
        (!farm.canWalk(CollisionBox{entity_dimensions::PLAYER}.at(data.position)) ||
         !farm.canWalk(CollisionBox{entity_dimensions::PLAYER}.at(data.returnPosition)))) {
        return false;
    }
    entrance = data;
    return true;
}

bool WorldManager::acceptsWorld(uint8_t worldId) const {
    return !loadedWorld || *loadedWorld == worldId;
}

bool WorldManager::canMove(const Player& player, const Position& next) const {
    const CollisionBounds bounds = player.getCollisionBounds(next);
    if (player.getZone() == Zone::Dungeon) {
        return dungeon.canWalk(player.getDungeonLevel(), bounds);
    }
    if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        return farm.canWalk(bounds);
    }
    return !entrance || player.getZone() != entrance->zone || entrance->terrain.isWalkable(bounds);
}

void WorldManager::completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const {
    data.zoneData = std::monostate{};
    if (player.getZone() == Zone::Farm) {
        data.zoneData = farm.getPublicData();
        return;
    }
    if (player.getZone() == Zone::Dungeon) {
        const auto mine = dungeon.getMineData(player.getDungeonLevel(), includeBroken);
        if (mine) {
            data.zoneData = *mine;
        }
    }
}

void WorldManager::interact(Player& player, Direction direction) {
    if (usePassage(player)) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    if (player.getZone() == Zone::Dungeon) {
        dungeon.useSelectedItem(player, direction, now);
    } else if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        farm.useSelectedItem(player, direction, entityIds, now);
    }
}

bool WorldManager::usePassage(Player& player) {
    if (!loadedWorld || !entrance || !player.canAct()) {
        return false;
    }
    if (player.getZone() == Zone::Dungeon) {
        if (dungeon.canExit(player)) {
            if (entrance->zone == Zone::Farm && farm.isInitialized() &&
                !farm.canWalk(player.getCollisionBounds(entrance->returnPosition))) {
                return false;
            }
            if (!entrance->terrain.isWalkable(player.getCollisionBounds(entrance->returnPosition))) {
                return false;
            }
            return player.changeLocation(entrance->zone, 0, entrance->returnPosition);
        }
        return dungeon.advanceRoom(player);
    }
    if (player.getZone() != entrance->zone ||!entrance->terrain.sameBaseCell(player.getPosition(), entrance->position)) {
        return false;
    }
    return dungeon.enter(player);
}

void WorldManager::afterStatusPublished() {
    dungeon.removeBrokenMinerals();
}

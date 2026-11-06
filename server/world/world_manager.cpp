#include "world_manager.h"
#include "player/player.h"

WorldManager::WorldManager(const DungeonConfig& config, uint32_t seed)
    : dungeon(config, seed) {}

bool WorldManager::loadDungeon(uint8_t worldId,const std::map<uint8_t, DungeonRoomLayout>& layout) {
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
    return !entrance || player.getZone() != entrance->zone || entrance->terrain.isWalkable(bounds);
}

void WorldManager::completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const {
    if (player.getZone() == Zone::Dungeon) {
        if (const auto mine = dungeon.getMineData(player.getDungeonLevel(), includeBroken)) {
            data.zoneData = *mine;
        }
    }
}

void WorldManager::interact(Player& player, Direction direction) {
    if (usePassage(player) || player.getZone() != Zone::Dungeon) {
        return;
    }
    dungeon.useSelectedItem(player, direction, std::chrono::steady_clock::now());
}

bool WorldManager::usePassage(Player& player) {
    if (!loadedWorld || !entrance || !player.canAct()) {
        return false;
    }
    if (player.getZone() == Zone::Dungeon) {
        if (dungeon.canExit(player)) {
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

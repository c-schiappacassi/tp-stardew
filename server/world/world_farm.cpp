#include "world_manager.h"

bool WorldManager::loadFarm(uint8_t worldId, const FarmLayout& layout) {
    if (loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    if (!farm.initialize(layout, entityIds)) {
        return false;
    }
    loadedWorld = worldId;
    return true;
}
//bool WorldManager::spawnPlayer(Player& player) const {
  //  return farm.spawnPlayer(player);
//}

const FarmManager& WorldManager::getFarm() const { return farm; }

void WorldManager::update(std::chrono::milliseconds elapsed,
    const std::vector<CollisionBounds>& farmPlayers) {
    if (!farm.isInitialized()) {
        return;
    }
    farm.setObstacles(farmPlayers);
    farm.update(elapsed);
}

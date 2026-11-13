#include "world_manager.h"

#include <utility>

#include "player/player.h"

WorldManager::WorldManager(const DungeonConfig& config, uint32_t seed, const FarmConfig& farmConfig)
    : dungeon(config, seed), farm(farmConfig, seed) {}


ConstMapData WorldManager::buildConstMap() const {
    ConstMapData map;

    for (const auto& t : farm.getBackground()) {
        const Position px(t.tile.getX() * 16, t.tile.getY() * 16);
        if (t.textureId > static_cast<uint16_t>(TileTextureId::Dirt)) {
            throw std::runtime_error("texture_id de granja desconocido");
        }
        map.farm.floor.push_back({px, static_cast<TileTextureId>(t.textureId)});
    }

    // TODO: decor / buildings de la granja (hoy FarmLayout no los tiene)
    // TODO: fondo de los niveles. Por ahora vacios, pero en orden: MinesView numera por indice (i + 1)
    map.levels.resize(DUNGEON_ROOM_COUNT);
    // TODO: farm_houses, pierres, robins, clints, marnies
    return map;
}

bool WorldManager::spawnPlayer(Player& player) const {
    return farm.isInitialized() ? farm.spawnPlayer(player) : dungeon.spawnPlayer(player);
}

bool WorldManager::recoverPlayer(Player& player) const { return spawnPlayer(player); }

bool WorldManager::loadDungeon(uint8_t worldId,const std::map<uint8_t, DungeonRoomLayout>& layout) {
    if (farm.isInitialized() && loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    if (!dungeon.initialize(layout, entityIds)) {
        return false;
    }
    loadedWorld = worldId;
    travel.resetDungeon();
    return true;
}

bool WorldManager::configureDungeonEntrance(const DungeonEntrance& data) {
    return loadedWorld && travel.configureEntrance(data, farm, town);
}

bool WorldManager::loadTown(const TownConfig& config) {
    TownMap prepared;
    if (!loadedWorld || !prepared.initialize(config) || !travel.canConnectTown(prepared, farm)) {
        return false;
    }
    town = std::move(prepared);
    return true;
}

void WorldManager::finishDay() {
    travel.finishDay();
    dungeon.update(std::chrono::milliseconds{0}, {}, entityIds, std::chrono::steady_clock::now());
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
    return player.getZone() == Zone::Town && town.canWalk(bounds);
}

void WorldManager::completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const {
    data.combatProgress = dungeon.getCombatProgress();
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

void WorldManager::interact(Player& player, Direction direction,uint16_t playerId) {
    if (!player.canAct()) return;
    const auto previousZone = player.getZone();
    if (travel.use(player, playerId, dungeon, farm, town)) {
        if (previousZone == Zone::Dungeon || player.getZone() == Zone::Dungeon) {
            dungeon.onTravel(playerId, player.getDungeonLevel(), player.canBeAttacked());
        }
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    if (player.getZone() == Zone::Dungeon) {
        dungeon.useSelectedItem(player, direction, now, entityIds, playerId);
    } else if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        if (!farm.useBed(player)) {
            farm.useSelectedItem(player, direction, entityIds, now);
        }
    }
}

bool WorldManager::combatCompleted() const { return dungeon.combatCompleted(); }

std::optional<DungeonCombatData> WorldManager::getCombatData(uint8_t number) const {
    return dungeon.getCombatData(number);
}

const CombatProgressData& WorldManager::getCombatProgress() const {
    return dungeon.getCombatProgress();
}

void WorldManager::afterStatusPublished() { 
    dungeon.removeBrokenMinerals(); 
}

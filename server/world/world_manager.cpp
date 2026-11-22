#include "world_manager.h"

#include <utility>

#include "chests/interactions/chest_interactions.h"
#include "player/player_manager.h"

WorldManager::WorldManager(const DungeonConfig& config, uint32_t seed, const FarmConfig& farmConfig):
        dungeon(config, seed), farm(farmConfig, seed), farmSeed(seed), animalRandom(seed) {}


ConstMapData WorldManager::buildConstMap() const {
    ConstMapData map;

    for (const auto& t: farm.getBackground()) {
        const Position px(t.tile.getX() * 16, t.tile.getY() * 16);
        if (t.textureId > static_cast<uint16_t>(TileTextureId::Dirt)) {
            throw std::runtime_error("unknown farm texture_id");
        }
        map.farm.floor.push_back({px, static_cast<TileTextureId>(t.textureId)});
    }

    // TODO: decor / buildings de la granja (hoy FarmLayout no los tiene)
    if (const auto entrance = travel.getDungeonEntrance(Zone::Farm)) {
        map.farm.portals.push_back({*entrance, PortalId::FarmToMine});
    }
    // TODO: fondo de los niveles. Por ahora vacios, pero en orden: MinesView numera por indice (i + 1)
    map.levels.resize(DUNGEON_ROOM_COUNT);
    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        map.levels[number - 1].portals = dungeon.getPortals(number);
    }
    // TODO: edificios y NPCs del pueblo cuando exista BuildingId/sprites para ellos
    const int townTile = town.getTileSize();
    map.town.floor.reserve(static_cast<size_t>(town.getWidth()) * town.getHeight());
    for (int y = 0; y < town.getHeight(); ++y) {
        for (int x = 0; x < town.getWidth(); ++x) {
            map.town.floor.push_back({Position(x * townTile, y * townTile), TileTextureId::Grass});
        }
    }
    interiors.completeMap(map);
    return map;
}

std::optional<WorldLocation> WorldManager::spawnFor(const PlayerSnapshot& player) const { return farm.isInitialized() ? farm.spawnFor(player) : dungeon.spawnFor(player); }

std::optional<WorldLocation> WorldManager::recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const {
    const auto house = interiors.recoveryFor(player, playerId);
    return house ? house : spawnFor(player);
}

bool WorldManager::loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout) {
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

bool WorldManager::configureDungeonEntrance(const DungeonEntrance& data) { return loadedWorld && travel.configureEntrance(data, farm, town); }

bool WorldManager::loadTown(const TownConfig& config) {
    TownMap prepared;
    if (!loadedWorld || !prepared.initialize(config) || !travel.canConnectTown(prepared, farm)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    std::vector<Position> townDoors;
    for (const auto& room: interiors.layouts()) {
        if (room.outside.zone != Zone::Town)
            continue;
        if (!prepared.canWalk(playerBox.at(room.outside.position)) || !prepared.canWalk(playerBox.at(room.returnPosition)) ||
            !prepared.getMap().hasBasePath(room.outside.position, room.returnPosition))
            return false;
        for (const auto destination: {Zone::Farm, Zone::Dungeon}) {
            const auto door = prepared.connectionTo(destination);
            if (door && prepared.getMap().sameBaseCell(*door, room.outside.position))
                return false;
        }
        for (const auto& door: townDoors) {
            if (prepared.getMap().sameBaseCell(door, room.outside.position))
                return false;
        }
        townDoors.push_back(room.outside.position);
    }
    town = std::move(prepared);
    return true;
}

void WorldManager::finishDay(PlayerManager& players, std::chrono::milliseconds remainingTime) {
    if (farm.isInitialized())
        farm.finishDay(remainingTime, entityIds);
    travel.finishDay();
    dungeon.update(std::chrono::milliseconds{0}, {}, players, entityIds, std::chrono::steady_clock::now());
}

bool WorldManager::acceptsWorld(uint8_t worldId) const { return !loadedWorld || *loadedWorld == worldId; }

bool WorldManager::canMove(const PlayerSnapshot& player, const Position& next) const {
    const CollisionBounds bounds = player.getCollisionBounds(next);
    if (player.getZone() == Zone::Dungeon) {
        return dungeon.canWalk(player.getDungeonLevel(), bounds);
    }
    if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        return farm.canWalk(bounds);
    }
    if (player.getZone() == Zone::InsideHouse && player.getInterior()) {
        return interiors.canWalk(*player.getInterior(), bounds);
    }
    return player.getZone() == Zone::Town && town.canWalk(bounds);
}

void WorldManager::completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const {
    data.combatProgress = dungeon.getCombatProgress();
    data.zoneData = std::monostate{};
    data.openChest.reset();
    if (player.getZone() == Zone::Farm) {
        data.zoneData = farm.getPublicData();
        data.openChest = farm.openChestFor(player);
        return;
    }
    if (player.getZone() == Zone::InsideHouse && player.getInterior()) {
        data.zoneData = interiors.dataFor(*player.getInterior());
        data.openChest = interiors.openChestFor(player);
        return;
    }
    if (player.getZone() == Zone::Dungeon) {
        const auto mine = dungeon.getMineData(player.getDungeonLevel(), includeBroken);
        if (mine) {
            data.zoneData = *mine;
            data.openChest = dungeon.openChestFor(player);
        }
    }
}

void WorldManager::interact(PlayerManager& players, uint16_t playerId, Direction direction) {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return;
    const auto previousZone = player.getZone();
    if (travel.use(players, playerId, dungeon, farm, town, interiors)) {
        const auto arrived = players.snapshot(playerId);
        if (previousZone == Zone::Dungeon || arrived.getZone() == Zone::Dungeon) {
            dungeon.onTravel(playerId, arrived.getDungeonLevel(), arrived.canBeAttacked());
        }
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    if (player.getZone() == Zone::Dungeon) {
        dungeon.useSelectedItem(players, playerId, direction, now, entityIds);
    } else if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        farm.useSelectedItem(players, playerId, direction, entityIds, now);
    } else if (player.getZone() == Zone::InsideHouse) {
        if (!interiors.interactWithChest(players, playerId, direction, entityIds, now))
            interiors.useBed(players, playerId);
    }
}

bool WorldManager::combatCompleted() const { return dungeon.combatCompleted(); }

std::optional<DungeonCombatData> WorldManager::getCombatData(uint8_t number) const { return dungeon.getCombatData(number); }

const CombatProgressData& WorldManager::getCombatProgress() const { return dungeon.getCombatProgress(); }

void WorldManager::afterStatusPublished() { dungeon.removeBrokenMinerals(); }

void WorldManager::restoreFarm(const PersistenceDTO& saved) {
    farm.restoreData(saved.farm, saved.cropGrowthProgress, saved.cropWaterRemainingTime, saved.harvestedCrops, saved.animalProductionProgress, saved.animalAvailableProducts,
        saved.regrowingCrops, saved.emptySoilElapsed, saved.chestInventories);
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

#include "world_manager.h"

#include "loading/map_converter.h"
#include "player/player_manager.h"

WorldManager::WorldManager(const DungeonConfig& config, uint32_t seed, const FarmConfig& farmConfig):
    dungeon(config, seed), farm(farmConfig, seed), animalRandom(seed),
    travel(dungeon, farm, town, interiors, entityIds), loading(dungeon, farm, town, interiors, entityIds, travel, seed),
    queries(dungeon, farm, town, interiors, travel), interactions(dungeon, farm, interiors, entityIds), persistence(dungeon, farm, interiors, entityIds) {}

PreparedWorld WorldManager::prepareMap(const MapData& data, uint8_t worldId,
                                      const InteriorTemplates& interiors) {
    const MapConverter converter;
    return {converter.convert(data, worldId, interiors), converter.convertVisuals(data)};
}

bool WorldManager::loadFarm(uint8_t worldId, const FarmLayout& layout) { return loading.loadFarm(worldId, layout); }

bool WorldManager::loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout) { return loading.loadDungeon(worldId, layout); }

bool WorldManager::loadTown(const TownConfig& config) { return loading.loadTown(config); }

bool WorldManager::configureDungeonEntrance(const DungeonEntrance& data) { return loading.configureDungeonEntrance(data); }

bool WorldManager::acceptsWorld(uint8_t worldId) const { return loading.acceptsWorld(worldId); }

ConstMapData WorldManager::buildConstMap() const { return queries.buildConstMap(); }

std::optional<WorldLocation> WorldManager::spawnFor(const PlayerSnapshot& player) const {
    return farm.isInitialized() ? farm.spawnFor(player) : dungeon.spawnFor(player);
}

std::optional<WorldLocation> WorldManager::recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const {
    const auto house = interiors.recoveryFor(player, playerId);
    return house ? house : spawnFor(player);
}

bool WorldManager::canMove(const PlayerSnapshot& player, const Position& next) const { return queries.canMove(player, next); }

void WorldManager::completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const {
    queries.completeZoneData(player, data, includeBroken);
}

std::optional<NpcData> WorldManager::nearbyMerchant(const PlayerSnapshot& player, Direction direction) const { return interiors.nearbyMerchant(player, direction); }

FarmPrices WorldManager::farmPrices() const { return farm.prices(); }

std::optional<uint32_t> WorldManager::animalBasePrice(AnimalSpecies species) const { return farm.animalBasePrice(species); }

std::optional<uint32_t> WorldManager::productBasePrice(ItemId item) const { return farm.productBasePrice(item); }

bool WorldManager::buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant) {
    return farm.buyAnimals(species, quantity, variant, entityIds, animalRandom);
}

bool WorldManager::sellAnimals(AnimalSpecies species, uint16_t quantity) { return farm.sellAnimals(species, quantity); }

void WorldManager::update(std::chrono::milliseconds elapsed, PlayerManager& players, std::chrono::steady_clock::time_point now) {
    if (farm.isInitialized()) {
        farm.setObstacles(players.getCollisionBounds(Zone::Farm));
        farm.update(elapsed, entityIds);
    }
    const auto playerIds = players.connectedPlayerIds();
    dungeon.update(elapsed, players.combatants(), players, entityIds, now);
    interactions.collectNearbyItems(players, playerIds);
}

void WorldManager::finishDay(PlayerManager& players, std::chrono::milliseconds remainingTime) {
    if (farm.isInitialized())
        farm.finishDay(remainingTime, entityIds);
    travel.finishDay();
    dungeon.update(std::chrono::milliseconds{0}, {}, players, entityIds, std::chrono::steady_clock::now());
}

void WorldManager::interact(PlayerManager& players, uint16_t playerId, Direction direction) {
    if (!travel.use(players, playerId))
        interactions.useSelectedItem(players, playerId, direction, std::chrono::steady_clock::now());
}

bool WorldManager::teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level, std::optional<InteriorLocation> inside) {
    return travel.teleport(players, playerId, destination, level, inside);
}

void WorldManager::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) { interactions.depositInChest(players, playerId, command); }

void WorldManager::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) { interactions.withdrawFromChest(players, playerId, command); }

bool WorldManager::combatCompleted() const { return dungeon.combatCompleted(); }

std::optional<DungeonCombatData> WorldManager::getCombatData(uint8_t room) const { return dungeon.getCombatData(room); }

CombatProgressData WorldManager::getCombatProgress() const { return dungeon.getCombatProgress(); }

void WorldManager::afterStatusPublished() { dungeon.removeBrokenMinerals(); }

void WorldManager::restoreFarm(const PersistenceDTO& saved) { persistence.restoreFarm(saved); }

void WorldManager::saveFarmAndStorage(PersistenceDTO& saved) const { persistence.saveFarmAndStorage(saved); }

std::size_t WorldManager::careCrops() { return farm.careCrops(); }

std::size_t WorldManager::careAnimals() { return farm.careAnimals(entityIds); }

bool WorldManager::spawnEnemies(const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, const std::vector<Position>& occupied) {
    return dungeon.spawnEnemies(players, playerId, type, quantity, pairs, entityIds, occupied);
}

uint16_t WorldManager::killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly) { return dungeon.killEnemies(players, playerId, nearestOnly, entityIds); }

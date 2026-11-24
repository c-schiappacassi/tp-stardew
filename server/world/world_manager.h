#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <vector>

#include "dungeon/contracts/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "entities/entity_id_generator.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "farm/queries/farm_prices.h"
#include "interiors/interior_rooms.h"
#include "persistence/persistence_dto.h"
#include "player/ports/movement_rules.h"
#include "player/ports/player_interactions.h"
#include "player/ports/player_spawn_source.h"
#include "player/ports/zone_data_source.h"
#include "town/town_map.h"

#include "contracts/world_setup.h"
#include "interactions/world_interactions.h"
#include "loading/world_loader.h"
#include "persistence/world_persistence.h"
#include "queries/world_queries.h"
#include "travel/world_travel.h"

struct MapData;
class PlayerManager;
class PlayerSnapshot;

class WorldManager final: public MovementRules, public ZoneDataSource, public PlayerInteractions, public PlayerSpawnSource {
private:
    DungeonManager dungeon;
    FarmManager farm;
    std::mt19937 animalRandom;
    TownMap town;
    InteriorRooms interiors;
    EntityIdGenerator entityIds;
    WorldTravel travel;
    WorldLoader loading;
    WorldQueries queries;
    WorldInteractions interactions;
    WorldPersistence persistence;

public:
    explicit WorldManager(const DungeonConfig& config = {}, uint32_t seed = std::random_device{}(), const FarmConfig& farmConfig = FarmConfig{});

    // Collaborators reference owned members, so the manager must keep a stable address.
    WorldManager(const WorldManager&) = delete;
    WorldManager& operator=(const WorldManager&) = delete;
    WorldManager(WorldManager&&) = delete;
    WorldManager& operator=(WorldManager&&) = delete;

    static PreparedWorld prepareMap(const MapData& data, uint8_t worldId);
    bool loadFarm(uint8_t worldId, const FarmLayout& layout);
    bool loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool loadTown(const TownConfig& config);
    bool configureDungeonEntrance(const DungeonEntrance& data);
    bool acceptsWorld(uint8_t worldId) const;

    ConstMapData buildConstMap() const;
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const override;
    std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const override;
    bool canMove(const PlayerSnapshot& player, const Position& next) const override;
    void completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const override;
    std::optional<NpcData> nearbyMerchant(const PlayerSnapshot& player, Direction direction = Direction::None) const;
    FarmPrices farmPrices() const;
    std::optional<uint32_t> animalBasePrice(AnimalSpecies species) const;
    std::optional<uint32_t> productBasePrice(ItemId item) const;
    bool buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant = std::nullopt);
    bool sellAnimals(AnimalSpecies species, uint16_t quantity);

    void update(std::chrono::milliseconds elapsed, PlayerManager& players, std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    void finishDay(PlayerManager& players, std::chrono::milliseconds remainingTime = std::chrono::milliseconds{0});
    void afterStatusPublished();
    void interact(PlayerManager& players, uint16_t playerId, Direction direction) override;
    bool teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level = 0, std::optional<InteriorLocation> inside = std::nullopt);
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) override;
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) override;

    bool combatCompleted() const;
    std::optional<DungeonCombatData> getCombatData(uint8_t room) const;
    CombatProgressData getCombatProgress() const;
    void restoreFarm(const PersistenceDTO& saved);
    void saveFarmAndStorage(PersistenceDTO& saved) const;

    std::size_t careCrops();
    std::size_t careAnimals();
    bool spawnEnemies(const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, const std::vector<Position>& occupied = {});
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly);
};

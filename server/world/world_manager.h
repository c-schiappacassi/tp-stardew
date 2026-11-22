#pragma once

#include <optional>
#include <random>

#include "dungeon/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "entities/entity_id_generator.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "interiors/interior_manager.h"
#include "persistence/persistence_dto.h"
#include "player/ports/movement_rules.h"
#include "player/ports/player_interactions.h"
#include "player/ports/player_spawn_source.h"
#include "player/ports/zone_data_source.h"
#include "town/town_map.h"

#include "world_travel.h"

class PlayerManager;
class PlayerSnapshot;

class WorldManager final: public MovementRules, public ZoneDataSource, public PlayerInteractions, public PlayerSpawnSource {
private:
    DungeonManager dungeon;
    FarmManager farm;
    uint32_t farmSeed;
    std::mt19937 animalRandom;
    TownMap town;
    InteriorManager interiors;
    EntityIdGenerator entityIds;
    std::optional<uint8_t> loadedWorld;
    WorldTravel travel;

public:
    explicit WorldManager(const DungeonConfig& config = {}, uint32_t seed = std::random_device{}(), const FarmConfig& farmConfig = FarmConfig{});
    ConstMapData buildConstMap() const;
    bool loadFarm(uint8_t worldId, const FarmLayout& layout);
    bool loadTown(const TownConfig& config);
    void finishDay(PlayerManager& players, std::chrono::milliseconds remainingTime = std::chrono::milliseconds{0});
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const override;
    std::optional<WorldLocation> recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const override;
    const FarmManager& getFarm() const;
    void restoreFarm(const PersistenceDTO& saved);
    void saveFarmAndStorage(PersistenceDTO& saved) const;
    std::optional<NpcData> nearbyMerchant(const PlayerSnapshot& player, Direction direction = Direction::None) const;
    bool buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant = std::nullopt);
    bool sellAnimals(AnimalSpecies species, uint16_t quantity);
    void update(std::chrono::milliseconds elapsed, PlayerManager& players, std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    bool combatCompleted() const;
    std::optional<DungeonCombatData> getCombatData(uint8_t room) const;
    const CombatProgressData& getCombatProgress() const;
    bool loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool configureDungeonEntrance(const DungeonEntrance& data);
    bool acceptsWorld(uint8_t worldId) const;
    bool canMove(const PlayerSnapshot& player, const Position& next) const override;
    void completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const override;
    void interact(PlayerManager& players, uint16_t playerId, Direction direction) override;
    void depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) override;
    void withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) override;
    void afterStatusPublished();
    bool teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level = 0, std::optional<InteriorLocation> inside = std::nullopt);
    std::size_t careCrops();
    std::size_t careAnimals();
    bool spawnEnemies(const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, const std::vector<Position>& occupied = {});
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly);
};

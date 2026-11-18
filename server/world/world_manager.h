#pragma once

#include <optional>

#include "dungeon/dungeon_entrance.h"
#include "dungeon/dungeon_manager.h"
#include "entities/entity_id_generator.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "player/ports/movement_rules.h"
#include "player/ports/player_interactions.h"
#include "player/ports/player_spawn_source.h"
#include "player/ports/zone_data_source.h"
#include "world_travel.h"
#include "interiors/interior_manager.h"
#include "town/town_map.h"

class WorldManager final : public MovementRules, public ZoneDataSource, public PlayerInteractions,public PlayerSpawnSource {
private:
    DungeonManager dungeon;
    FarmManager farm;
    uint32_t farmSeed;
    TownMap town;
    InteriorManager interiors;
    EntityIdGenerator entityIds;
    std::optional<uint8_t> loadedWorld;
    WorldTravel travel;

public:
    explicit WorldManager(const DungeonConfig& config = {},uint32_t seed = std::random_device{}(),const FarmConfig& farmConfig = FarmConfig{});
    ConstMapData buildConstMap() const;
    bool loadFarm(uint8_t worldId, const FarmLayout& layout);
    bool loadTown(const TownConfig& config);
    void finishDay(std::chrono::milliseconds remainingTime = std::chrono::milliseconds{0});
    bool spawnPlayer(Player& player) const override;
    bool recoverPlayer(Player& player, uint16_t playerId = 0) const override;
    const FarmManager& getFarm() const;
    std::optional<NpcData> nearbyMerchant(const Player& player, Direction direction = Direction::None) const;
    bool buyAnimals(AnimalSpecies species, uint16_t quantity);
    bool sellAnimals(AnimalSpecies species, uint16_t quantity);
    void update(std::chrono::milliseconds elapsed, const std::vector<CollisionBounds>& farmPlayers = {}, const std::vector<Combatant>& players = {},std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());
    bool combatCompleted() const;
    std::optional<DungeonCombatData> getCombatData(uint8_t room) const;
    const CombatProgressData& getCombatProgress() const;
    bool loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool configureDungeonEntrance(const DungeonEntrance& data);
    bool acceptsWorld(uint8_t worldId) const;
    bool canMove(const Player& player, const Position& next) const override;
    void completeZoneData(const Player& player, PlayerDataLocal& data,bool includeBroken) const override;
    void interact(Player& player, Direction direction,uint16_t playerId = 0) override;
    void afterStatusPublished();
};

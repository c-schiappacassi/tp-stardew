#pragma once

#include <optional>
#include "dungeon/dungeon_manager.h"
#include "dungeon/dungeon_entrance.h"
#include "entities/entity_id_generator.h"
#include "player/ports/movement_rules.h"
#include "player/ports/zone_data_source.h"
#include "player/ports/player_interactions.h"
#include "player/ports/player_spawn_source.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"

class WorldManager final : public MovementRules, public ZoneDataSource, public PlayerInteractions,
                           public PlayerSpawnSource {
private:
    DungeonManager dungeon;
    FarmManager farm;
    EntityIdGenerator entityIds;
    std::optional<uint8_t> loadedWorld;
    std::optional<DungeonEntrance> entrance;
    bool usePassage(Player& player);

public:
    explicit WorldManager(const DungeonConfig& config = {},uint32_t seed = std::random_device{}(),const FarmConfig& farmConfig = FarmConfig{});
    bool loadFarm(uint8_t worldId, const FarmLayout& layout);
    bool spawnPlayer(Player& player) const override;
    const FarmManager& getFarm() const;
    void update(std::chrono::milliseconds elapsed, const std::vector<CollisionBounds>& farmPlayers = {});
    bool loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout);
    bool configureDungeonEntrance(const DungeonEntrance& data);
    bool acceptsWorld(uint8_t worldId) const;
    bool canMove(const Player& player, const Position& next) const override;
    void completeZoneData(const Player& player,PlayerDataLocal& data,bool includeBroken) const override;
    void interact(Player& player, Direction direction) override;
    void afterStatusPublished();
};

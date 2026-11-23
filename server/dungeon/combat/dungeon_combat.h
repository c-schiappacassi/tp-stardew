#pragma once

#include <map>
#include <vector>

#include "dungeon/contracts/dungeon_layout.h"
#include "dungeon/dungeon_room.h"
#include "dungeon/enemies/enemy_generator.h"
#include "player/ports/combatant.h"

#include "combat_progress.h"

class DungeonRooms;
class PlayerManager;
class PlayerSnapshot;

class DungeonCombat {
private:
    using PlayersByRoom = std::map<uint8_t, std::vector<Combatant>>;

    EnemyConfig config;
    EnemyGenerator generator;
    std::mt19937 trapRandom;
    CombatProgress progress;
    CombatTime clock{0};
    uint32_t visit = 0;
    bool occupied = false;
    bool visitEnded = false;
    std::map<uint16_t, uint32_t> lastVisits;
    std::map<uint16_t, uint8_t> presence;
    std::map<uint8_t, DungeonRoomLayout> layouts;

    bool regenerate(DungeonRooms& rooms, EntityIdGenerator& ids);
    void endVisit();
    bool prepareVisit(const PlayerManager& players, uint16_t playerId, DungeonRooms& rooms, EntityIdGenerator& ids);
    bool spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied);
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, DungeonRoom& room, bool nearestOnly, EntityIdGenerator& ids);
    PlayersByRoom updatePresence(const std::vector<Combatant>& combatants, PlayerManager& players, std::map<uint16_t, uint8_t> current, DungeonRooms& rooms);
    void spawnPendingAmbush(DungeonRoom& room, const std::vector<Combatant>& combatants, EntityIdGenerator& ids);

public:
    DungeonCombat(const EnemyConfig& config, uint32_t seed);
    bool initialize(DungeonRooms& rooms, const std::map<uint8_t, DungeonRoomLayout>& layouts, EntityIdGenerator& ids);
    bool attack(PlayerManager& players, uint16_t playerId, DungeonRoom& room, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void silverMined(DungeonRoom& room);
    void update(CombatTime elapsed, DungeonRooms& rooms, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids,
        std::chrono::steady_clock::time_point now);
    void onTravel(uint16_t id, uint8_t room, bool survived, const DungeonRooms& rooms);
    bool spawnEnemies(const PlayerManager& players, uint16_t playerId, DungeonRooms& rooms, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids,
        const std::vector<Position>& occupied);
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, DungeonRooms& rooms, bool nearestOnly, EntityIdGenerator& ids);
    const CombatProgressData& getProgress() const;
    bool completed() const;
    uint32_t getVisit() const;
};

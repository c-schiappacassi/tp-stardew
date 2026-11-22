#pragma once

#include <map>
#include <vector>

#include "dungeon/enemies/enemy_generator.h"
#include "dungeon/loading/dungeon_layout.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/ports/combatant.h"

#include "combat_progress.h"

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

    bool regenerate(std::map<uint8_t, DungeonRoom>& rooms, EntityIdGenerator& ids);
    void endVisit();
    PlayersByRoom updatePresence(const std::vector<Combatant>& combatants, PlayerManager& players, std::map<uint16_t, uint8_t> current, std::map<uint8_t, DungeonRoom>& rooms);
    void spawnPendingAmbush(DungeonRoom& room, const std::vector<Combatant>& combatants, EntityIdGenerator& ids);

public:
    DungeonCombat(const EnemyConfig& config, uint32_t seed);
    bool initialize(std::map<uint8_t, DungeonRoom>& rooms, const std::map<uint8_t, DungeonRoomLayout>& layouts, EntityIdGenerator& ids);
    bool attack(PlayerManager& players, uint16_t playerId, DungeonRoom& room, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    void silverMined(DungeonRoom& room);
    void update(CombatTime elapsed, std::map<uint8_t, DungeonRoom>& rooms, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids,
        std::chrono::steady_clock::time_point now);
    void onTravel(uint16_t id, uint8_t room, bool survived, const std::map<uint8_t, DungeonRoom>& rooms);
    bool spawnEnemies(DungeonRoom& room, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied);
    bool prepareVisit(const PlayerManager& players, uint16_t playerId, std::map<uint8_t, DungeonRoom>& rooms, EntityIdGenerator& ids);
    uint16_t killEnemies(PlayerManager& players, uint16_t playerId, DungeonRoom& room, bool nearestOnly, EntityIdGenerator& ids);
    const CombatProgressData& getProgress() const;
    bool completed() const;
    uint32_t getVisit() const;
};

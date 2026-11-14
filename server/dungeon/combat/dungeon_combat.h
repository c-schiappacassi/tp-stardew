#pragma once

#include <map>
#include <vector>

#include "dungeon/enemies/enemy_generator.h"
#include "dungeon/loading/dungeon_layout.h"
#include "dungeon/rooms/dungeon_room.h"
#include "player/ports/combatant.h"

#include "combat_progress.h"

class Player;

class DungeonCombat {
private:
    using PlayersByRoom = std::map<uint8_t, std::vector<Combatant>>;

    EnemyConfig config;
    EnemyGenerator generator;
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
    PlayersByRoom updatePresence(const std::vector<Combatant>& players,std::map<uint16_t, uint8_t> current,std::map<uint8_t, DungeonRoom>& rooms);
    void spawnPendingAmbush(DungeonRoom& room, const std::vector<Combatant>& players,EntityIdGenerator& ids);

public:
    DungeonCombat(const EnemyConfig& config, uint32_t seed);
    bool initialize(std::map<uint8_t, DungeonRoom>& rooms,const std::map<uint8_t, DungeonRoomLayout>& layouts, EntityIdGenerator& ids);
    bool attack(Player& player, uint16_t id, DungeonRoom& room, Direction direction,EntityIdGenerator& ids, std::chrono::steady_clock::time_point now);
    bool collectLoot(Player& player, DungeonRoom& room);
    void silverMined(DungeonRoom& room);
    void update(CombatTime elapsed, std::map<uint8_t, DungeonRoom>& rooms,const std::vector<Combatant>& players, EntityIdGenerator& ids,std::chrono::steady_clock::time_point now);
    void onTravel(uint16_t id, uint8_t room, bool survived,const std::map<uint8_t, DungeonRoom>& rooms);
    const CombatProgressData& getProgress() const;
    bool completed() const;
    uint32_t getVisit() const;
};

#pragma once

#include <map>
#include <set>

#include "dungeon/enemies/enemy.h"

class CombatProgress {
    struct Evidence {
        std::set<uint8_t> skeletonRooms;
        std::map<uint32_t, std::set<uint32_t>> pairs;
        std::map<uint32_t, std::set<uint32_t>> groups;
        uint8_t room = 0;
        bool sawSkull = false;
        bool damaged = false;
    };
    std::map<uint16_t, Evidence> players;
    CombatProgressData completed;

public:
    void newVisit();
    void defeat(uint16_t playerId, uint8_t room, const Enemy& enemy);
    void enter(uint16_t playerId, uint8_t room, bool skull);
    void damage(uint16_t playerId);
    void leave(uint16_t playerId, bool survived);
    const CombatProgressData& data() const;
    bool complete() const;
};

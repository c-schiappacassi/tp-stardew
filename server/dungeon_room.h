#pragma once

#include <cstdint>
#include <map>
#include "mineral.h"

class DungeonRoom {
private:
    uint8_t number;
    std::map<EntityId, Mineral> minerals;

public:
    explicit DungeonRoom(uint8_t number);

    uint8_t getNumber() const;
    bool addMineral(Mineral mineral);
    bool removeMineral(EntityId id);
    Mineral* findMineral(EntityId id);
    const Mineral* findMineral(EntityId id) const;
    const std::map<EntityId, Mineral>& getMinerals() const;
};
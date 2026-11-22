#pragma once

#include <chrono>

#include "farm_targeting.h"

class EntityIdGenerator;
class FarmManager;
class PlayerManager;
class PlayerSnapshot;

class FarmInteractions {
private:
    FarmTargeting targeting;

    bool useShovel(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction, EntityIdGenerator& entityIds) const;
    bool waterCrop(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction) const;
    bool collectProduct(const PlayerManager& players, uint16_t playerId, FarmManager& farm, Direction direction, EntityIdGenerator& entityIds) const;
    bool placeSelectedItem(PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction, ItemId item) const;
    bool applySelectedItem(PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction) const;

public:
    bool interact(PlayerManager& players, uint16_t playerId, FarmManager& farm, EntityIdGenerator& entityIds, Direction direction, std::chrono::steady_clock::time_point now) const;
};

#pragma once

#include <chrono>

#include "farm_targeting.h"

class EntityIdGenerator;
class FarmManager;
class Player;

class FarmInteractions {
private:
    FarmTargeting targeting;

    bool useShovel(Player& player, FarmManager& farm, Direction direction,EntityIdGenerator& entityIds) const;
    bool waterCrop(Player& player, FarmManager& farm, Direction direction) const;
    bool collectProduct(Player& player, FarmManager& farm, Direction direction,EntityIdGenerator& entityIds) const;
    bool placeSelectedItem(Player& player, FarmManager& farm, EntityIdGenerator& entityIds,
        Direction direction, ItemId item) const;
    bool applySelectedItem(Player& player, FarmManager& farm, EntityIdGenerator& entityIds,
        Direction direction) const;

public:
    bool interact(Player& player, FarmManager& farm, EntityIdGenerator& entityIds,
        Direction direction, std::chrono::steady_clock::time_point now) const;
};

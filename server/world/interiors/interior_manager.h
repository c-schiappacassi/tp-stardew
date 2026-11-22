#pragma once

#include "interior_layout.h"

class Player;

class InteriorManager {
    std::vector<InteriorLayout> rooms;

public:
    bool initialize(const std::vector<InteriorLayout>& layouts);
    const InteriorLayout* find(const InteriorLocation& location) const;
    const std::vector<InteriorLayout>& layouts() const { return rooms; }
    bool canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const;
    bool useBed(Player& player) const;
    bool recoverPlayer(Player& player, uint16_t playerId) const;
    InteriorData dataFor(const InteriorLocation& location) const;
    void completeMap(ConstMapData& map) const;
};

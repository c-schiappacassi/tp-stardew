#pragma once

#include <cstdint>
#include <optional>

#include "entities/collision_box.h"
#include "world/zone_map.h"

#include "protocol_types.h"

class PlayerSnapshot;

class InteractionTargeting {
public:
    bool hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const;
    bool canReach(const PlayerSnapshot& player, const Position& position, Direction direction, const ZoneMap& map, uint16_t range = 32) const;
    std::optional<Position> findAdjacentPosition(const PlayerSnapshot& player, Direction direction, const ZoneMap& map, const CollisionBox& collision, uint16_t range = 32) const;
};

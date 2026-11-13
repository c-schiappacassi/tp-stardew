#pragma once

#include <optional>

#include "protocol_types.h"
#include "world/zone_map.h"

struct TownConfig;

class TownMap {
private:
    ZoneMap terrain;
    std::optional<Position> farmDoor;
    std::optional<Position> mineDoor;

public:
    bool initialize(const TownConfig& config);
    const ZoneMap& getMap() const;
    bool canWalk(const CollisionBounds& bounds) const;
    std::optional<Position> connectionTo(Zone destination) const;
};

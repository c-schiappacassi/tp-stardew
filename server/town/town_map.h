#pragma once

#include <cstdint>
#include <optional>

#include "protocol_types.h"
#include "world/zone_map.h"

struct TownConfig;

class TownMap {
private:
    ZoneMap terrain;
    std::optional<Position> farmDoor;
    std::optional<Position> mineDoor;
    uint16_t widthTiles = 0;
    uint16_t heightTiles = 0;
    uint16_t tileSizePx = 0;

public:
    bool initialize(const TownConfig& config);
    const ZoneMap& getMap() const;
    bool canWalk(const CollisionBounds& bounds) const;
    std::optional<Position> connectionTo(Zone destination) const;
    uint16_t getWidth() const { return widthTiles; }
    uint16_t getHeight() const { return heightTiles; }
    uint16_t getTileSize() const { return tileSizePx; }
};

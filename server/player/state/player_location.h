#pragma once

#include <cstdint>
#include <optional>

#include "dtos.h"

class PlayerLocation {
private:
    Position position;
    Zone zone;
    uint8_t dungeonLevel;
    std::optional<InteriorLocation> interior;

public:

    explicit PlayerLocation(const PlayerData& data);

    const Position& getPosition() const;
    Zone getZone() const;
    uint8_t getDungeonLevel() const;
    const std::optional<InteriorLocation>& getInterior() const;
    Position nextPosition(Direction direction, int step) const;
    void advance(Direction direction, int step);
    bool relocate(Zone destination, uint8_t level, const Position& target,
                  std::optional<InteriorLocation> inside);
    bool canReach(const Position& target, uint16_t range) const;
    std::optional<Direction> directionTo(const Position& target) const;
};

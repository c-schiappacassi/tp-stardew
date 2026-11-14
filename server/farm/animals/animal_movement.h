#pragma once

#include <cstdint>
#include <optional>
#include <random>

#include "animal_movement_config.h"
#include "farm/ports/farm_placement.h"
#include "protocol_types.h"

class AnimalMovement {
private:
    Position position;
    AnimalMovementConfig config;
    Direction direction = Direction::Down;
    Direction movementDirection = Direction::None;

    uint32_t ticksUntilDecision = 0;
    bool walking = false;

    std::optional<Position> findDestination(EntityId id, const FarmPlacement& placement) const;

public:
    AnimalMovement(const Position& position, const AnimalMovementConfig& config);

    const Position& getPosition() const;
    Direction getDirection() const;
    bool isWalking() const;

    bool moveTo(EntityId id, const Position& next, FarmPlacement& placement);
    void tick(EntityId id, FarmPlacement& placement, std::mt19937& random);
};

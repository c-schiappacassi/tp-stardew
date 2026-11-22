#include "animal_movement.h"

#include <limits>
#include <stdexcept>

AnimalMovement::AnimalMovement(const Position& position, const AnimalMovementConfig& config):
    position(position), config(config) {
    config.validate();
    if (position.getX() < 0 || position.getY() < 0) {
        throw std::invalid_argument("Invalid animal position");
    }
}

const Position& AnimalMovement::getPosition() const { return position; }

Direction AnimalMovement::getDirection() const { return direction; }

bool AnimalMovement::isWalking() const { return walking; }

std::optional<Position> AnimalMovement::findDestination(EntityId id,
    const Placement& placement) const {
    int dx = 0;
    int dy = 0;

    switch (movementDirection) {
        case Direction::Up:
            dy = -1;
            break;

        case Direction::Down:
            dy = 1;
            break;

        case Direction::Left:
            dx = -1;
            break;

        case Direction::Right:
            dx = 1;
            break;

        case Direction::None:
            return std::nullopt;
    }

    int64_t x = position.getX();
    int64_t y = position.getY();

    for (int travelled = 0; travelled < config.stepPerTick; ++travelled) {
        x += dx;
        y += dy;

        if (x < 0 || y < 0 || x > std::numeric_limits<int>::max() ||
            y > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }

        const Position next{static_cast<int>(x), static_cast<int>(y)};

        if (!placement.isAvailable(next, id)) {
            return std::nullopt;
        }
    }

    return Position{static_cast<int>(x), static_cast<int>(y)};
}

bool AnimalMovement::moveTo(EntityId id, const Position& next, Placement& placement) {
    if (!placement.move(id, next)) {
        return false;
    }

    position = next;
    walking = false;
    movementDirection = Direction::None;
    ticksUntilDecision = 0;

    return true;
}

void AnimalMovement::tick(EntityId id, Placement& placement, std::mt19937& random) {
    walking = false;

    if (!placement.hasMap()) {
        return;
    }

    if (ticksUntilDecision == 0) {
        constexpr Direction choices[]{Direction::None, Direction::Up, Direction::Down,
            Direction::Left, Direction::Right};

        std::uniform_int_distribution<int> choice{0, 4};

        movementDirection = choices[choice(random)];
        ticksUntilDecision = config.decisionIntervalTicks;
    }

    --ticksUntilDecision;

    if (movementDirection == Direction::None) {
        return;
    }

    direction = movementDirection;

    const auto next = findDestination(id, placement);

    if (!next || !placement.move(id, *next)) {
        movementDirection = Direction::None;
        return;
    }

    position = *next;
    walking = true;
}

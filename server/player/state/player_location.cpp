#include "player_location.h"

#include <algorithm>
#include <cmath>

PlayerLocation::PlayerLocation(const PlayerData& data):
        position(data.pos),
        zone(data.currentZone),
        dungeonLevel(data.dungeonLevel),
        interior(data.interior) {}

const Position& PlayerLocation::getPosition() const { return position; }

Zone PlayerLocation::getZone() const { return zone; }

uint8_t PlayerLocation::getDungeonLevel() const { return dungeonLevel; }

const std::optional<InteriorLocation>& PlayerLocation::getInterior() const { return interior; }

Position PlayerLocation::nextPosition(Direction direction, int step) const {
    if (step <= 0)
        return position;
    int x = position.getX();
    int y = position.getY();
    switch (direction) {
        case Direction::Up:
            y -= step;
            break;
        case Direction::Down:
            y += step;
            break;
        case Direction::Left:
            x -= step;
            break;
        case Direction::Right:
            x += step;
            break;
        default:
            return position;
    }
    return Position{std::max(0, x), std::max(0, y)};
}

void PlayerLocation::advance(Direction direction, int step) {
    position = nextPosition(direction, step);
}

bool PlayerLocation::relocate(Zone destination, uint8_t level, const Position& target,std::optional<InteriorLocation> inside) {
    if (target.getX() < 0 || target.getY() < 0)
        return false;
    if ((destination == Zone::InsideHouse) != inside.has_value() ||
        (inside && inside->interior_id > InteriorId::ComunityCenter))
        return false;

    if (destination == Zone::Dungeon) {
        if (level == 0)
            return false;
    } else if (destination == Zone::Farm || destination == Zone::Town ||
               destination == Zone::InsideHouse) {
        if (level != 0)
            return false;
    } else {
        return false;
    }

    position = target;
    zone = destination;
    dungeonLevel = level;
    interior = inside;
    return true;
}

bool PlayerLocation::canReach(const Position& target, uint16_t range) const {
    const double dx = static_cast<double>(position.getX()) - target.getX();
    const double dy = static_cast<double>(position.getY()) - target.getY();
    return dx * dx + dy * dy <= static_cast<double>(range) * range;
}

std::optional<Direction> PlayerLocation::directionTo(const Position& target) const {
    const double dx = static_cast<double>(target.getX()) - position.getX();
    const double dy = static_cast<double>(target.getY()) - position.getY();
    if (dx == 0 && dy == 0)
        return std::nullopt;
    if (std::abs(dx) >= std::abs(dy))
        return dx > 0 ? Direction::Right : Direction::Left;
    return dy > 0 ? Direction::Down : Direction::Up;
}

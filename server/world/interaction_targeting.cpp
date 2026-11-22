#include "interaction_targeting.h"

#include <algorithm>
#include <cmath>

#include "player/queries/player_snapshot.h"

bool InteractionTargeting::hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const {
    const int dx = target.getX() - origin.getX();
    const int dy = target.getY() - origin.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    for (int step = 1; step <= steps; ++step) {
        const int xOffset = static_cast<int>(std::round(static_cast<double>(dx) * step / steps));
        const int yOffset = static_cast<int>(std::round(static_cast<double>(dy) * step / steps));
        const Position sample{origin.getX() + xOffset, origin.getY() - 1 + yOffset};
        if (!map.isWalkable(sample) || !map.isWalkable(Position{sample.getX(), previous.getY()}) || !map.isWalkable(Position{previous.getX(), sample.getY()})) {
            return false;
        }
        previous = sample;
    }
    return true;
}

bool InteractionTargeting::canReach(const PlayerSnapshot& player, const Position& position, Direction direction, const ZoneMap& map, uint16_t range) const {
    if (!player.canReach(position, range)) {
        return false;
    }
    const int dx = position.getX() - player.getPosition().getX();
    const int dy = position.getY() - player.getPosition().getY();
    const bool inDirection = (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) || (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) ||
        (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) || (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    return inDirection && hasClearPath(player.getPosition(), position, map);
}

std::optional<Position> InteractionTargeting::findAdjacentPosition(
    const PlayerSnapshot& player, Direction direction, const ZoneMap& map, const CollisionBox& collision, uint16_t range) const {
    auto cell = map.baseCellAt(player.getPosition());
    if (!cell) {
        return std::nullopt;
    }
    switch (direction) {
        case Direction::Up:
            --cell->first;
            break;
        case Direction::Down:
            ++cell->first;
            break;
        case Direction::Left:
            --cell->second;
            break;
        case Direction::Right:
            ++cell->second;
            break;
        default:
            return std::nullopt;
    }
    const auto position = map.baseAt(*cell);
    if (!position || !canReach(player, *position, direction, map, range) || player.getCollisionBounds(player.getPosition()).intersects(collision.at(*position))) {
        return std::nullopt;
    }
    return position;
}

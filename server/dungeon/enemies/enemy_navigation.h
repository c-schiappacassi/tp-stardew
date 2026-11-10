#pragma once

#include <optional>

#include "position.h"
#include "protocol_types.h"

class DungeonRoom;

namespace enemy_navigation {
std::optional<Position> nextWaypoint(const DungeonRoom& room, const Position& from,
                                     const Position& target);
double squaredDistance(const Position& first, const Position& second);
Direction facing(const Position& from, const Position& target);
bool isInFront(Direction direction, const Position& from, const Position& target);
Position offsetPosition(const Position& position, Direction direction, int distance);
}

#pragma once

#include <optional>

#include "position.h"

struct DungeonRoomPassages {
    Position entry{0, 0};
    Position exit{0, 0};
    std::optional<Position> nextRoom;
};
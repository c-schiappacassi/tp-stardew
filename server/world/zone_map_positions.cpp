#include "zone_map.h"

#include <limits>
#include <stdexcept>

std::vector<Position> ZoneMap::walkablePositions() const {
    std::vector<Position> positions;
    if (isEmpty()) {
        return positions;
    }

    const auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max() / cellSize);

    for (std::size_t row = 0; row < walkableTiles.size(); ++row) {
        for (std::size_t column = 0; column < walkableTiles[row].size(); ++column) {
            if (!walkableTiles[row][column]) {
                continue;
            }
            if (row > limit || column > limit) {
                throw std::overflow_error("El terreno excede el rango de Position");
            }

            positions.emplace_back(static_cast<int>(column) * cellSize,static_cast<int>(row) * cellSize);
        }
    }
    return positions;
}
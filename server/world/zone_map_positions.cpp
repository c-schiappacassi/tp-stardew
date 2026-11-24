#include "zone_map.h"

#include <limits>
#include <stdexcept>

std::optional<ZoneMap::Cell> ZoneMap::baseCellAt(const Position& base) const {
    if (base.getY() <= 0) return std::nullopt;
    return cellAt(Position{base.getX(), base.getY() - 1});
}

std::optional<Position> ZoneMap::baseAt(const Cell& cell) const {
    if (isEmpty() || cell.first < 0 || cell.second < 0 ||
        static_cast<std::size_t>(cell.first) >= walkableTiles.size() ||
        static_cast<std::size_t>(cell.second) >= walkableTiles.front().size()) {
        return std::nullopt;
    }
    const int64_t x = static_cast<int64_t>(cell.second) * cellSize + cellSize / 2;
    const int64_t y = (static_cast<int64_t>(cell.first) + 1) * cellSize;
    if (x > std::numeric_limits<int>::max() || y > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }
    return Position{static_cast<int>(x), static_cast<int>(y)};
}

bool ZoneMap::isWalkableBase(const Position& base) const {
    const auto cell = baseCellAt(base);
    return cell && walkableTiles[cell->first][cell->second];
}

bool ZoneMap::sameBaseCell(const Position& first, const Position& second) const {
    const auto a = baseCellAt(first);
    const auto b = baseCellAt(second);
    return a && b && *a == *b;
}

bool ZoneMap::hasBasePath(const Position& from, const Position& to) const {
    return from.getY() > 0 && to.getY() > 0 &&
           hasPath(Position{from.getX(), from.getY() - 1},
                   Position{to.getX(), to.getY() - 1});
}

std::vector<Position> ZoneMap::walkableBasePositions() const {
    std::vector<Position> result;
    if (isEmpty()) return result;
    for (std::size_t row = 0; row < walkableTiles.size(); ++row) {
        for (std::size_t col = 0; col < walkableTiles[row].size(); ++col) {
            if (!walkableTiles[row][col]) continue;
            if (row > static_cast<std::size_t>(std::numeric_limits<int>::max()) || col > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                throw std::overflow_error("Cell outside Position range");
            }
            const auto base = baseAt({static_cast<int>(row), static_cast<int>(col)});
            if (!base) throw std::overflow_error("Base outside Position range");
            result.push_back(*base);
        }
    }
    return result;
}

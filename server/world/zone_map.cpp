#include "zone_map.h"

#include <cstddef>
#include <stdexcept>
#include <queue>
#include <limits>

ZoneMap::ZoneMap(const std::vector<std::vector<bool>>& walkableTiles,int cellSize):walkableTiles(walkableTiles),cellSize(cellSize) {
    if (cellSize <= 0) {
        throw std::invalid_argument("El tamano de celda debe ser positivo");
    }
    if (walkableTiles.empty() || walkableTiles.front().empty()) {
        throw std::invalid_argument("El mapa no puede estar vacio");
    }

    const std::size_t columns = walkableTiles.front().size();

    for (const auto& row : walkableTiles) {
        if (row.size() != columns) {
            throw std::invalid_argument("El mapa debe ser rectangular");
        }
    }
}

bool ZoneMap::contains(const Position& pos) const {
    if (cellSize <= 0 || pos.getX()< 0 || pos.getY()< 0 ||
        walkableTiles.empty()) {
        return false;
    }

    const std::size_t column = pos.getX() /cellSize;
    const std::size_t row = pos.getY() /cellSize;

    return row < walkableTiles.size() &&column < walkableTiles.front().size();
}

bool ZoneMap::isWalkable(const Position& pos) const {
    if (!contains(pos)) {
        return false;
    }
    const int column = pos.getX()/cellSize;
    const int row = pos.getY()/ cellSize;

    return walkableTiles[row][column];
}

std::optional<ZoneMap::CellRange> ZoneMap::coveredCells(const CollisionBounds& bounds) const {
    if (bounds.left < 0 || bounds.top < 0 || bounds.right < bounds.left ||bounds.bottom < bounds.top || bounds.right > std::numeric_limits<int>::max() ||bounds.bottom > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }
    const auto first = cellAt(Position(static_cast<int>(bounds.left), static_cast<int>(bounds.top)));
    const auto last = cellAt(Position(static_cast<int>(bounds.right), static_cast<int>(bounds.bottom)));
    if (!first || !last) return std::nullopt;
    return CellRange{*first, *last};
}

bool ZoneMap::isWalkable(const CollisionBounds& bounds) const {
    const auto cells = coveredCells(bounds);
    if (!cells) return false;
    for (int row = cells->first.first; row <= cells->last.first; ++row) {
        for (int column = cells->first.second; column <= cells->last.second; ++column) {
            if (!walkableTiles[row][column]) return false;
        }
    }
    return true;
}

bool ZoneMap::sameCell(const Position& first, const Position& second) const {
    if (!contains(first) || !contains(second)) {
        return false;
    }

    return first.getX() / cellSize==second.getX()/ cellSize && first.getY() /cellSize == second.getY() / cellSize;
}

bool ZoneMap::isEmpty() const {
    return walkableTiles.empty();
}

std::optional<ZoneMap::Cell> ZoneMap::cellAt(
    const Position& pos) const {
    if (!contains(pos)) {
        return std::nullopt;
    }

    return Cell{
        pos.getY()/cellSize,
        pos.getX()/cellSize
    };
}

bool ZoneMap::hasPath(const Position& from, const Position& to) const {
    if (!isWalkable(from) || !isWalkable(to)) {
        return false;
    }

    const Cell start = *cellAt(from);
    const Cell goal = *cellAt(to);
    std::vector<std::vector<bool>> visited(walkableTiles.size(),std::vector<bool>(walkableTiles.front().size(), false));
    std::queue<Cell> pending;
    pending.push(start);
    visited[start.first][start.second] = true;

    constexpr Cell directions[] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    while (!pending.empty()) {
        const Cell current = pending.front();
        pending.pop();

        if (current == goal) {
            return true;
        }

        for (const auto& [dr, dc] : directions) {
            const int row = current.first + dr;
            const int col = current.second + dc;

            if (row < 0 || col < 0 ||static_cast<std::size_t>(row) >= walkableTiles.size() ||static_cast<std::size_t>(col) >= walkableTiles.front().size()) {
                continue;
            }

            if (!walkableTiles[row][col] || visited[row][col]) {
                continue;
            }

            visited[row][col] = true;
            pending.emplace(row, col);
        }
    }

    return false;
}

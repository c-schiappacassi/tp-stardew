#include "zone_map.h"

#include <cstddef>
#include <stdexcept>

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
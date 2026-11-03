#pragma once

#include <vector>
#include "position.h"
#include <optional>
#include <utility>
#include "entities/collision_box.h"

class ZoneMap {
private:
    std::vector<std::vector<bool>> walkableTiles;
    int cellSize = 0;

public:
    ZoneMap() = default;

    ZoneMap(const std::vector<std::vector<bool>>& walkableTiles,int cellSize);

    using Cell = std::pair<int, int>;
    struct CellRange {
        Cell first;
        Cell last;
    };
    std::optional<CellRange> coveredCells(const CollisionBounds& bounds) const;
    bool isEmpty() const;
    std::optional<Cell> cellAt(const Position& pos) const;
    bool contains(const Position& pos) const;
    bool isWalkable(const Position& pos) const;
    bool isWalkable(const CollisionBounds& bounds) const;
    bool sameCell(const Position& first, const Position& second) const;
    bool hasPath(const Position& from, const Position& to) const;
    std::vector<Position> walkablePositions() const;
};

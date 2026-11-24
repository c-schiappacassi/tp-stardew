#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "entities/collision_bounds.h"
#include "entities/collision_box.h"
#include "position.h"
#include "protocol_types.h"

class ZoneMap {
private:
    std::vector<std::vector<bool>> walkableTiles;
    int cellSize = 0;

public:
    ZoneMap();

    ZoneMap(const std::vector<std::vector<bool>>& walkableTiles, int cellSize);

    using Cell = std::pair<int, int>;
    struct CellRange {
        Cell first;
        Cell last;
    };
    std::optional<CellRange> coveredCells(const CollisionBounds& bounds) const;
    bool isEmpty() const;
    int getCellSize() const;
    std::optional<Cell> cellAt(const Position& pos) const;
    bool contains(const Position& pos) const;
    bool isWalkable(const Position& pos) const;
    bool isWalkable(const CollisionBounds& bounds) const;
    bool sameCell(const Position& first, const Position& second) const;
    bool hasPath(const Position& from, const Position& to) const;
    std::optional<Cell> baseCellAt(const Position& base) const;
    std::optional<Position> baseAt(const Cell& cell) const;
    bool isWalkableBase(const Position& base) const;
    bool sameBaseCell(const Position& first, const Position& second) const;
    bool hasBasePath(const Position& from, const Position& to) const;
    std::vector<Position> walkableBasePositions() const;
    bool hasClearPath(const Position& origin, const Position& target) const;
    bool canReach(const Position& origin, const Position& target, Direction direction, uint16_t range) const;
    std::optional<Position> findAdjacentPosition(const Position& origin, Direction direction, const CollisionBounds& actorBounds,const CollisionBox& collision, uint16_t range = 32) const;
};

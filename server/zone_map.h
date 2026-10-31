#pragma once

#include <vector>
#include "position.h"
#include <optional>
#include <utility>

class ZoneMap {
private:
    std::vector<std::vector<bool>> walkableTiles;
    int cellSize = 0;

public:
    ZoneMap() = default;

    ZoneMap(const std::vector<std::vector<bool>>& walkableTiles,int cellSize);

    using Cell = std::pair<int, int>;
    bool isEmpty() const;
    std::optional<Cell> cellAt(const Position& pos) const;
    bool contains(const Position& pos) const;
    bool isWalkable(const Position& pos) const;
    bool sameCell(const Position& first, const Position& second) const;
};
#include "zone_map.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <queue>
#include <stdexcept>

ZoneMap::ZoneMap() = default;

int ZoneMap::getCellSize() const { return cellSize; }

ZoneMap::ZoneMap(const std::vector<std::vector<bool>>& walkableTiles, int cellSize): walkableTiles(walkableTiles), cellSize(cellSize) {
    if (cellSize <= 0) {
        throw std::invalid_argument("Cell size must be positive");
    }
    if (walkableTiles.empty() || walkableTiles.front().empty()) {
        throw std::invalid_argument("The map cannot be empty");
    }

    const std::size_t columns = walkableTiles.front().size();

    for (const auto& row: walkableTiles) {
        if (row.size() != columns) {
            throw std::invalid_argument("The map must be rectangular");
        }
    }
}

bool ZoneMap::contains(const Position& pos) const {
    if (cellSize <= 0 || pos.getX() < 0 || pos.getY() < 0 || walkableTiles.empty()) {
        return false;
    }

    const std::size_t column = pos.getX() / cellSize;
    const std::size_t row = pos.getY() / cellSize;

    return row < walkableTiles.size() && column < walkableTiles.front().size();
}

bool ZoneMap::isWalkable(const Position& pos) const {
    if (!contains(pos)) {
        return false;
    }
    const int column = pos.getX() / cellSize;
    const int row = pos.getY() / cellSize;

    return walkableTiles[row][column];
}

std::optional<ZoneMap::CellRange> ZoneMap::coveredCells(const CollisionBounds& bounds) const {
    if (!bounds.isValid() || bounds.getLeft() < 0 || bounds.getTop() < 0 ||
        bounds.getRight() > std::numeric_limits<int>::max() || bounds.getBottom() > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }
    const auto first = cellAt(Position(static_cast<int>(bounds.getLeft()), static_cast<int>(bounds.getTop())));
    const auto last = cellAt(Position(static_cast<int>(bounds.getRight()), static_cast<int>(bounds.getBottom())));
    if (!first || !last)
        return std::nullopt;
    return CellRange{*first, *last};
}

bool ZoneMap::isWalkable(const CollisionBounds& bounds) const {
    const auto cells = coveredCells(bounds);
    if (!cells)
        return false;
    for (int row = cells->first.first; row <= cells->last.first; ++row) {
        for (int column = cells->first.second; column <= cells->last.second; ++column) {
            if (!walkableTiles[row][column])
                return false;
        }
    }
    return true;
}

bool ZoneMap::sameCell(const Position& first, const Position& second) const {
    if (!contains(first) || !contains(second)) {
        return false;
    }

    return first.getX() / cellSize == second.getX() / cellSize && first.getY() / cellSize == second.getY() / cellSize;
}

bool ZoneMap::isEmpty() const { return walkableTiles.empty(); }

std::optional<ZoneMap::Cell> ZoneMap::cellAt(const Position& pos) const {
    if (!contains(pos)) {
        return std::nullopt;
    }

    return Cell{pos.getY() / cellSize, pos.getX() / cellSize};
}

bool ZoneMap::hasPath(const Position& from, const Position& to) const {
    if (!isWalkable(from) || !isWalkable(to)) {
        return false;
    }

    const Cell start = *cellAt(from);
    const Cell goal = *cellAt(to);
    std::vector<std::vector<bool>> visited(walkableTiles.size(), std::vector<bool>(walkableTiles.front().size(), false));
    std::queue<Cell> pending;
    pending.push(start);
    visited[start.first][start.second] = true;

    constexpr Cell directions[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (!pending.empty()) {
        const Cell current = pending.front();
        pending.pop();

        if (current == goal) {
            return true;
        }

        for (const auto& [dr, dc]: directions) {
            const int row = current.first + dr;
            const int col = current.second + dc;

            if (row < 0 || col < 0 || static_cast<std::size_t>(row) >= walkableTiles.size() || static_cast<std::size_t>(col) >= walkableTiles.front().size()) {
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

std::optional<ZoneMap::Cell> ZoneMap::baseCellAt(const Position& base) const {
    if (base.getY() <= 0)
        return std::nullopt;
    return cellAt(Position{base.getX(), base.getY() - 1});
}

std::optional<Position> ZoneMap::baseAt(const Cell& cell) const {
    if (isEmpty() || cell.first < 0 || cell.second < 0 || static_cast<std::size_t>(cell.first) >= walkableTiles.size() ||
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
    return from.getY() > 0 && to.getY() > 0 && hasPath(Position{from.getX(), from.getY() - 1}, Position{to.getX(), to.getY() - 1});
}

std::vector<Position> ZoneMap::walkableBasePositions() const {
    std::vector<Position> result;
    if (isEmpty())
        return result;
    for (std::size_t row = 0; row < walkableTiles.size(); ++row) {
        for (std::size_t col = 0; col < walkableTiles[row].size(); ++col) {
            if (!walkableTiles[row][col])
                continue;
            if (row > static_cast<std::size_t>(std::numeric_limits<int>::max()) || col > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                throw std::overflow_error("Cell outside Position range");
            }
            const auto base = baseAt({static_cast<int>(row), static_cast<int>(col)});
            if (!base)
                throw std::overflow_error("Base outside Position range");
            result.push_back(*base);
        }
    }
    return result;
}

bool ZoneMap::hasClearPath(const Position& origin, const Position& target) const {
    const int dx = target.getX() - origin.getX();
    const int dy = target.getY() - origin.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    for (int step = 1; step <= steps; ++step) {
        const int xOffset = static_cast<int>(std::round(static_cast<double>(dx) * step / steps));
        const int yOffset = static_cast<int>(std::round(static_cast<double>(dy) * step / steps));
        const Position sample{origin.getX() + xOffset, origin.getY() - 1 + yOffset};
        if (!isWalkable(sample) || !isWalkable(Position{sample.getX(), previous.getY()}) || !isWalkable(Position{previous.getX(), sample.getY()})) {
            return false;
        }
        previous = sample;
    }
    return true;
}

bool ZoneMap::canReach(const Position& origin, const Position& position, Direction direction, uint16_t range) const {
    const double distanceX = static_cast<double>(origin.getX()) - position.getX();
    const double distanceY = static_cast<double>(origin.getY()) - position.getY();
    if (distanceX * distanceX + distanceY * distanceY > static_cast<double>(range) * range) {
        return false;
    }
    const int dx = position.getX() - origin.getX();
    const int dy = position.getY() - origin.getY();
    const bool inDirection = (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) || (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) ||
        (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) || (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    return inDirection && hasClearPath(origin, position);
}

std::optional<Position> ZoneMap::findAdjacentPosition(const Position& origin, Direction direction, const CollisionBounds& actorBounds,
    const CollisionBox& collision, uint16_t range) const {
    auto cell = baseCellAt(origin);
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
    const auto position = baseAt(*cell);
    if (!position || !canReach(origin, *position, direction, range) || actorBounds.intersects(collision.at(*position))) {
        return std::nullopt;
    }
    return position;
}

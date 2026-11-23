#include "enemy_navigation.h"

#include <cmath>
#include <map>
#include <queue>

#include "dungeon/dungeon_room.h"

#include "protocol_types.h"

double enemy_navigation::squaredDistance(const Position& a, const Position& b) {
    const double dx = static_cast<double>(a.getX()) - b.getX();
    const double dy = static_cast<double>(a.getY()) - b.getY();
    return dx * dx + dy * dy;
}

Direction enemy_navigation::facing(const Position& from, const Position& target) {
    const int dx = target.getX() - from.getX();
    const int dy = target.getY() - from.getY();
    if (std::abs(dx) > std::abs(dy)) {
        if (dx < 0) {
            return Direction::Left;
        }
        return Direction::Right;
    }
    if (dy < 0) {
        return Direction::Up;
    }
    return Direction::Down;
}

bool enemy_navigation::isInFront(Direction direction, const Position& from, const Position& target) {
    const int dx = target.getX() - from.getX();
    const int dy = target.getY() - from.getY();
    if (dx == 0 && dy == 0) {
        return true;
    }
    switch (direction) {
        case Direction::Up:
            return dy < 0 && std::abs(dx) <= -dy;
        case Direction::Down:
            return dy > 0 && std::abs(dx) <= dy;
        case Direction::Left:
            return dx < 0 && std::abs(dy) <= -dx;
        case Direction::Right:
            return dx > 0 && std::abs(dy) <= dx;
        default:
            return false;
    }
}

Position enemy_navigation::offsetPosition(const Position& position, Direction direction, int distance) {
    switch (direction) {
        case Direction::Up:
            return Position{position.getX(), position.getY() - distance};
        case Direction::Down:
            return Position{position.getX(), position.getY() + distance};
        case Direction::Left:
            return Position{position.getX() - distance, position.getY()};
        case Direction::Right:
            return Position{position.getX() + distance, position.getY()};
        default:
            return position;
    }
}

std::optional<Position> enemy_navigation::nextWaypoint(const DungeonRoom& room, const Position& from, const Position& target) {
    const auto& map = room.getMap();
    const auto start = map.baseCellAt(from);
    const auto goal = map.baseCellAt(target);
    if (!start || !goal) {
        return std::nullopt;
    }
    if (*start == *goal) {
        return target;
    }
    std::queue<ZoneMap::Cell> pending;
    std::map<ZoneMap::Cell, ZoneMap::Cell> parent;
    parent.emplace(*start, *start);
    pending.push(*start);
    constexpr ZoneMap::Cell directions[]{{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    while (!pending.empty()) {
        const auto current = pending.front();
        pending.pop();
        if (current == *goal) {
            auto next = current;
            while (parent.at(next) != *start) {
                next = parent.at(next);
            }
            return map.baseAt(next);
        }
        for (const auto& [dy, dx]: directions) {
            const ZoneMap::Cell next{current.first + dy, current.second + dx};
            if (parent.contains(next)) {
                continue;
            }
            const auto pos = map.baseAt(next);
            if (!pos || !room.canWalk(CollisionBox{12, 8}.at(*pos))) {
                continue;
            }
            parent.emplace(next, current);
            pending.push(next);
        }
    }
    return std::nullopt;
}

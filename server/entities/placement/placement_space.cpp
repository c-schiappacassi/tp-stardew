#include "placement_space.h"

const ZoneMap& PlacementSpace::getMap() const { return terrain; }

bool PlacementSpace::hasMap() const { return !terrain.isEmpty(); }

bool PlacementSpace::contains(EntityId id) const { return occupants.contains(id); }

bool PlacementSpace::setMap(const ZoneMap& map) {
    if (map.isEmpty() || !occupants.empty()) {
        return false;
    }
    terrain = map;
    reservedAreas.clear();
    obstacles.clear();
    return true;
}

void PlacementSpace::setObstacles(const std::vector<CollisionBounds>& bounds) { obstacles = bounds; }

bool PlacementSpace::setReservedPositions(const std::vector<Position>& positions) {
    if (!occupants.empty() || terrain.isEmpty()) {
        return false;
    }
    std::vector<CollisionBounds> prepared;
    for (const auto& base : positions) {
        const auto bounds = CollisionBox{entity_dimensions::CROP}.at(base);
        if (!terrain.isWalkable(bounds)) {
            return false;
        }
        prepared.push_back(bounds);
    }
    reservedAreas = std::move(prepared);
    return true;
}

std::optional<ZoneMap::Cell> PlacementSpace::cellAt(const Position& base) const {
    if (base.getX() < 0 || base.getY() <= 0) {
        return std::nullopt;
    }
    if (!terrain.isEmpty()) {
        return terrain.baseCellAt(base);
    }
    return ZoneMap::Cell{(base.getY() - 1) / entity_dimensions::TILE_SIZE,
        base.getX() / entity_dimensions::TILE_SIZE};
}

bool PlacementSpace::isAvailable(const CollisionBounds& bounds, EntityId ignoredId) const {
    if (!bounds.isValid() || bounds.getLeft() < 0 || bounds.getTop() < 0 ||
        (!terrain.isEmpty() && !terrain.isWalkable(bounds))) {
        return false;
    }
    for (const auto& obstacle : obstacles) {
        if (obstacle.intersects(bounds)) {
            return false;
        }
    }
    for (const auto& area : reservedAreas) {
        if (area.intersects(bounds)) {
            return false;
        }
    }
    for (const auto& [id, occupant] : occupants) {
        if (id != ignoredId && occupant.collision.at(occupant.base).intersects(bounds)) {
            return false;
        }
    }
    return true;
}

bool PlacementSpace::isAvailable(const Position& base, EntityId movingId) const {
    const auto it = occupants.find(movingId);
    return it != occupants.end() && isAvailable(it->second.collision.at(base), movingId);
}

bool PlacementSpace::canWalk(const CollisionBounds& bounds) const {
    if (!bounds.isValid() || bounds.getLeft() < 0 || bounds.getTop() < 0 ||
        (!terrain.isEmpty() && !terrain.isWalkable(bounds))) {
        return false;
    }
    for (const auto& [id, occupant] : occupants) {
        if (occupant.blocksMovement && occupant.collision.at(occupant.base).intersects(bounds)) {
            return false;
        }
    }
    return true;
}

bool PlacementSpace::reserve(EntityId id, const Position& base, const CollisionBox& collision,
    bool blocksMovement) {
    if (id == 0 || contains(id) || !collision.isValid() ||
        !isAvailable(collision.at(base))) {
        return false;
    }
    return occupants.emplace(id, Occupant{base, collision, blocksMovement}).second;
}

bool PlacementSpace::move(EntityId id, const Position& base) {
    const auto it = occupants.find(id);
    if (it == occupants.end() || !isAvailable(base, id)) {
        return false;
    }
    it->second.base = base;
    return true;
}

bool PlacementSpace::release(EntityId id) {
    return occupants.erase(id) != 0;
}

void PlacementSpace::clearOccupants() { occupants.clear(); }

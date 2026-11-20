#pragma once

#include <map>
#include <vector>
#include "farm/ports/farm_placement.h"

class FarmSpace final : public FarmPlacement {
private:
    struct Occupant {
        Position base;
        CollisionBox collision;
        bool blocksMovement;
    };
    ZoneMap terrain;
    std::map<EntityId, Occupant> occupants;
    std::vector<CollisionBounds> reservedAreas;
    std::vector<CollisionBounds> obstacles;

public:
    bool setMap(const ZoneMap& map);
    void setObstacles(const std::vector<CollisionBounds>& bounds);
    bool setReservedPositions(const std::vector<Position>& positions);
    const ZoneMap& getMap() const;
    bool hasMap() const override;
    bool contains(EntityId id) const override;
    bool canWalk(const CollisionBounds& bounds) const;
    std::optional<ZoneMap::Cell> cellAt(const Position& base) const override;
    bool isAvailable(const Position& base, EntityId movingId) const override;
    bool isAvailable(const CollisionBounds& bounds, EntityId ignoredId = 0) const override;
    bool reserve(EntityId id, const Position& base, const CollisionBox& collision,
        bool blocksMovement = true) override;
    bool move(EntityId id, const Position& base) override;
    bool release(EntityId id) override;
    void clearOccupants();
};

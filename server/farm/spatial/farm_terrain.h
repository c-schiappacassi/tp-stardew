#pragma once

#include <set>

#include "entities/placement/placement_space.h"
#include "farm/loading/farm_layout.h"
#include "world/world_location.h"

class PlayerSnapshot;

class FarmTerrain {
private:
    PlacementSpace space;
    bool initialized = false;
    Position spawnPosition{0, 0};
    std::vector<FarmBackgroundTile> background;
    std::set<ZoneMap::Cell> dirtCells;

public:
    bool initialize(const FarmLayout& layout);
    bool isInitialized() const;
    bool setMap(const ZoneMap& map);
    bool setReservedPositions(const std::vector<Position>& positions);
    void setObstacles(const std::vector<CollisionBounds>& bounds);
    bool canWalk(const CollisionBounds& bounds) const;
    bool isCultivable(const Position& position) const;
    bool sameBaseCell(const Position& first, const Position& second) const;
    std::optional<WorldLocation> spawnFor(const PlayerSnapshot& player) const;
    std::vector<TileData> backgroundData() const;

    PlacementSpace& getSpace();
    const PlacementSpace& getSpace() const;
    const ZoneMap& getMap() const;
};

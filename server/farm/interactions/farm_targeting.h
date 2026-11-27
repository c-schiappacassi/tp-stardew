#pragma once

#include <optional>

#include "entities/collision_box.h"
#include "entities/entity_id.h"
#include "farm/farm_product.h"

#include "protocol_types.h"

class Crop;
class FarmTerrain;
class CropManager;
class AnimalManager;
class PlayerSnapshot;
class ZoneMap;

enum class FarmProductSource { Crop, Animal };

struct FarmCollectionTarget {
    EntityId id;
    FarmProductSource source;
    Position position;
    FarmProduct product;
};

class FarmTargeting {
private:
    static int64_t squaredDistance(const Position& origin, const Position& target);
    bool canReach(const PlayerSnapshot& player, const Position& position, Direction direction, const ZoneMap& map) const;

public:
    bool hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const;
    std::optional<Position> findAdjacentPosition(const PlayerSnapshot& player, Direction direction, const ZoneMap& map, const CollisionBox& collision) const;
    const Crop* findCropToWater(const PlayerSnapshot& player, const FarmTerrain& terrain, const CropManager& crops, Direction direction) const;
    std::optional<FarmCollectionTarget> findCollectionTarget(const PlayerSnapshot& player, const FarmTerrain& terrain,const CropManager& crops, const AnimalManager& animals, Direction direction) const;
};

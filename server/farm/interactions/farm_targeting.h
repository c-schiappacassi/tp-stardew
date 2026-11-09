#pragma once

#include <optional>

#include "entities/collision_box.h"
#include "entities/entity_id.h"
#include "farm/farm_product.h"
#include "protocol_types.h"

class Crop;
class FarmManager;
class Player;
class ZoneMap;

enum class FarmProductSource {
    Crop,
    Animal
};

struct FarmCollectionTarget {
    EntityId id;
    FarmProductSource source;
    FarmProduct product;
};

class FarmTargeting {
private:
    bool canReach(const Player& player, const Position& position, Direction direction,
        const ZoneMap& map) const;
    bool hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const;

public:
    std::optional<Position> findAdjacentPosition(const Player& player, Direction direction,const ZoneMap& map, const CollisionBox& collision) const;
    const Crop* findCropToWater(const Player& player, const FarmManager& farm,Direction direction) const;
    std::optional<FarmCollectionTarget> findCollectionTarget(const Player& player,const FarmManager& farm, Direction direction) const;
};

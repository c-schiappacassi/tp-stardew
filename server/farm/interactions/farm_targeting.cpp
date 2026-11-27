#include "farm_targeting.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "farm/animals/animal_manager.h"
#include "farm/crops/crop_manager.h"
#include "farm/farm_constants.h"
#include "farm/spatial/farm_terrain.h"
#include "player/queries/player_snapshot.h"
#include "world/zone_map.h"

int64_t FarmTargeting::squaredDistance(const Position& origin, const Position& target) {
    const int64_t dx = static_cast<int64_t>(target.getX()) - origin.getX();
    const int64_t dy = static_cast<int64_t>(target.getY()) - origin.getY();
    return dx * dx + dy * dy;
}

bool FarmTargeting::hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const { return map.hasClearPath(origin, target); }

bool FarmTargeting::canReach(const PlayerSnapshot& player, const Position& position, Direction direction, const ZoneMap& map) const {
    return map.canReach(player.getPosition(), position, direction, farm_constants::INTERACTION_RANGE);
}

std::optional<Position> FarmTargeting::findAdjacentPosition(const PlayerSnapshot& player, Direction direction, const ZoneMap& map, const CollisionBox& collision) const {
    return map.findAdjacentPosition(player.getPosition(), direction, player.getCollisionBounds(player.getPosition()), collision, farm_constants::INTERACTION_RANGE);
}

const Crop* FarmTargeting::findCropToWater(const PlayerSnapshot& player, const FarmTerrain& terrain, const CropManager& crops, Direction direction) const {
    const Crop* closestCrop = nullptr;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    for (const auto& [id, crop]: crops.getCrops()) {
        if (crop.isReady() || crop.isWatered() || !canReach(player, crop.getPosition(), direction, terrain.getMap())) {
            continue;
        }
        const auto distance = squaredDistance(player.getPosition(), crop.getPosition());
        if (distance < closestDistance) {
            closestCrop = &crop;
            closestDistance = distance;
        }
    }
    return closestCrop;
}

std::optional<FarmCollectionTarget> FarmTargeting::findCollectionTarget(const PlayerSnapshot& player, const FarmTerrain& terrain,
    const CropManager& crops, const AnimalManager& animals, Direction direction) const {
    std::optional<FarmCollectionTarget> closestTarget;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    const auto considerProduct = [&](EntityId id, const Position& position, FarmProductSource source, const FarmProduct& product) {
        if (!canReach(player, position, direction, terrain.getMap())) {
            return;
        }
        const auto distance = squaredDistance(player.getPosition(), position);
        if (distance < closestDistance) {
            closestTarget = FarmCollectionTarget{id, source, position, product};
            closestDistance = distance;
        }
    };
    for (const auto& [id, crop]: crops.getCrops()) {
        const auto product = crop.getHarvest();
        if (product) {
            considerProduct(id, crop.getPosition(), FarmProductSource::Crop, *product);
        }
    }
    for (const auto& [id, animal]: animals.getAnimals()) {
        if (animal->dropsProductsAutomatically()) {
            continue;
        }
        const auto product = animal->getProduction();
        if (product) {
            considerProduct(animal->getId(), animal->getPosition(), FarmProductSource::Animal, *product);
        }
    }
    return closestTarget;
}

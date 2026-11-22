#include "farm_targeting.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "farm/farm_constants.h"
#include "farm/farm_manager.h"
#include "player/queries/player_snapshot.h"

namespace {
int64_t squaredDistance(const Position& origin, const Position& target) {
    const int64_t dx = static_cast<int64_t>(target.getX()) - origin.getX();
    const int64_t dy = static_cast<int64_t>(target.getY()) - origin.getY();
    return dx * dx + dy * dy;
}
}  // namespace

bool FarmTargeting::hasClearPath(const Position& origin, const Position& target, const ZoneMap& map) const {
    const int dx = target.getX() - origin.getX();
    const int dy = target.getY() - origin.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    for (int step = 1; step <= steps; ++step) {
        const int xOffset = static_cast<int>(std::round(static_cast<double>(dx) * step / steps));
        const int yOffset = static_cast<int>(std::round(static_cast<double>(dy) * step / steps));
        const Position sample{origin.getX() + xOffset, origin.getY() - 1 + yOffset};
        if (!map.isWalkable(sample) || !map.isWalkable(Position{sample.getX(), previous.getY()}) || !map.isWalkable(Position{previous.getX(), sample.getY()})) {
            return false;
        }
        previous = sample;
    }
    return true;
}

bool FarmTargeting::canReach(const PlayerSnapshot& player, const Position& position, Direction direction, const ZoneMap& map) const {
    if (!player.canReach(position, farm_constants::INTERACTION_RANGE)) {
        return false;
    }
    const int dx = position.getX() - player.getPosition().getX();
    const int dy = position.getY() - player.getPosition().getY();
    const bool inDirection = (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) || (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) ||
        (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) || (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    return inDirection && hasClearPath(player.getPosition(), position, map);
}

std::optional<Position> FarmTargeting::findAdjacentPosition(const PlayerSnapshot& player, Direction direction, const ZoneMap& map, const CollisionBox& collision) const {
    auto cell = map.baseCellAt(player.getPosition());
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
    const auto position = map.baseAt(*cell);
    if (!position || !canReach(player, *position, direction, map) || player.getCollisionBounds(player.getPosition()).intersects(collision.at(*position))) {
        return std::nullopt;
    }
    return position;
}

const Crop* FarmTargeting::findCropToWater(const PlayerSnapshot& player, const FarmManager& farm, Direction direction) const {
    const Crop* closestCrop = nullptr;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    for (const auto& [id, crop]: farm.getCrops()) {
        if (crop.isReady() || crop.isWatered() || !canReach(player, crop.getPosition(), direction, farm.getMap())) {
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

std::optional<FarmCollectionTarget> FarmTargeting::findCollectionTarget(const PlayerSnapshot& player, const FarmManager& farm, Direction direction) const {
    std::optional<FarmCollectionTarget> closestTarget;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    const auto considerProduct = [&](EntityId id, const Position& position, FarmProductSource source, const FarmProduct& product) {
        if (!canReach(player, position, direction, farm.getMap())) {
            return;
        }
        const auto distance = squaredDistance(player.getPosition(), position);
        if (distance < closestDistance) {
            closestTarget = FarmCollectionTarget{id, source, position, product};
            closestDistance = distance;
        }
    };
    for (const auto& [id, crop]: farm.getCrops()) {
        const auto product = crop.getHarvest();
        if (product) {
            considerProduct(id, crop.getPosition(), FarmProductSource::Crop, *product);
        }
    }
    for (const auto* animal: farm.getAnimals()) {
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

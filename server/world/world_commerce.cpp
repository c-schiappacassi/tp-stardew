#include <algorithm>
#include <cmath>

#include "player/queries/player_snapshot.h"

#include "world_manager.h"

std::optional<NpcData> WorldManager::nearbyMerchant(const PlayerSnapshot& player, Direction direction) const {
    if (!player.canAct() || player.getZone() != Zone::InsideHouse || !player.getInterior() || direction > Direction::Right)
        return std::nullopt;
    const auto* room = interiors.find(*player.getInterior());
    if (!room || !room->background.interior || !room->background.interior->npc)
        return std::nullopt;
    const auto& npc = *room->background.interior->npc;
    if (!player.canReach(npc.pos, 32))
        return std::nullopt;
    const auto& origin = player.getPosition();
    const int dx = npc.pos.getX() - origin.getX();
    const int dy = npc.pos.getY() - origin.getY();
    const bool facing = direction == Direction::None || (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) ||
        (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) || (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) ||
        (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    if (!facing)
        return std::nullopt;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    for (int step = 0; step <= steps; ++step) {
        const Position point{origin.getX() + dx * step / std::max(1, steps), origin.getY() - 1 + dy * step / std::max(1, steps)};
        if (!room->terrain.isWalkable(point) || !room->terrain.isWalkable(Position{point.getX(), previous.getY()}) ||
            !room->terrain.isWalkable(Position{previous.getX(), point.getY()}))
            return std::nullopt;
        previous = point;
    }
    return npc;
}

FarmPrices WorldManager::farmPrices() const { return farm.prices(); }

std::optional<uint32_t> WorldManager::animalBasePrice(AnimalSpecies species) const { return farm.animalBasePrice(species); }

std::optional<uint32_t> WorldManager::productBasePrice(ItemId item) const { return farm.productBasePrice(item); }

bool WorldManager::buyAnimals(AnimalSpecies species, uint16_t quantity, std::optional<AnimalType> variant) {
    return farm.buyAnimals(species, quantity, variant, entityIds, animalRandom);
}

bool WorldManager::sellAnimals(AnimalSpecies species, uint16_t quantity) { return farm.sellAnimals(species, quantity); }

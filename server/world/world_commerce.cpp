#include "world_manager.h"

#include <algorithm>
#include <cmath>

#include "player/player.h"

std::optional<NpcData> WorldManager::nearbyMerchant(const Player& player, Direction direction) const {
    if (!player.canAct() || player.getZone() != Zone::InsideHouse || !player.getInterior() ||
        direction > Direction::Right) return std::nullopt;
    const auto* room = interiors.find(*player.getInterior());
    if (!room || !room->background.interior || !room->background.interior->npc) return std::nullopt;
    const auto& npc = *room->background.interior->npc;
    if (!player.canReach(npc.pos, 32)) return std::nullopt;
    const auto& origin = player.getPosition();
    const int dx = npc.pos.getX() - origin.getX();
    const int dy = npc.pos.getY() - origin.getY();
    const bool facing = direction == Direction::None ||
        (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) ||
        (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) ||
        (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) ||
        (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    if (!facing) return std::nullopt;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    for (int step = 0; step <= steps; ++step) {
        const Position point{origin.getX() + dx * step / std::max(1, steps),
                             origin.getY() - 1 + dy * step / std::max(1, steps)};
        if (!room->terrain.isWalkable(point) ||
            !room->terrain.isWalkable(Position{point.getX(), previous.getY()}) ||
            !room->terrain.isWalkable(Position{previous.getX(), point.getY()})) return std::nullopt;
        previous = point;
    }
    return npc;
}

bool WorldManager::buyAnimals(AnimalSpecies species, uint16_t quantity) {
    if (!farm.isInitialized() || quantity == 0) return false;
    const auto& definitions = farm.getConfig().animals;
    const auto definition = std::find_if(definitions.begin(), definitions.end(), [&](const auto& entry) {
        return entry.species == species;
    });
    if (definition == definitions.end()) return false;
    const auto animals = farm.getAnimals();
    const auto count = static_cast<std::size_t>(std::count_if(animals.begin(), animals.end(),
        [&](const auto* animal) { return animal->getSpecies() == species; }));
    if (count > definition->maxCount || quantity > definition->maxCount - count) return false;
    std::vector<EntityId> added;
    added.reserve(quantity);
    auto preparedIds = entityIds;
    const auto rollback = [&] {
        for (const auto id : added) farm.removeAnimal(id);
    };
    const AnimalType type = species == AnimalSpecies::Cow ? AnimalType::BrownCow : AnimalType::BrownChicken;
    try {
        for (const auto& position : farm.getMap().walkableBasePositions()) {
            const auto id = preparedIds.getNextId();
            if (!farm.addAnimal(id, position, type)) continue;
            added.push_back(id);
            preparedIds.generate();
            if (added.size() == quantity) {
                entityIds = preparedIds;
                return true;
            }
        }
    } catch (...) {
        rollback();
        throw;
    }
    rollback();
    return false;
}

bool WorldManager::sellAnimals(AnimalSpecies species, uint16_t quantity) {
    if (!farm.isInitialized() || quantity == 0) return false;
    std::vector<EntityId> selected;
    for (const auto* animal : farm.getAnimals()) {
        if (animal->getSpecies() == species) selected.push_back(animal->getId());
        if (selected.size() == quantity) break;
    }
    if (selected.size() != quantity) return false;
    for (const auto id : selected) farm.removeAnimal(id);
    return true;
}

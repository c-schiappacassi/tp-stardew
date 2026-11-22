#include "farm_manager.h"

#include <algorithm>

std::size_t FarmManager::careCrops() {
    std::size_t count = 0;
    for (const auto& [id, crop] : cropManager.getCrops()) {
        if (crop.isHarvested()) continue;
        cropManager.findMutable(id)->growFully();
        ++count;
    }
    return count;
}

std::size_t FarmManager::careAnimals(EntityIdGenerator& entityIds) {
    const auto animals = animalManager.getAnimals();
    for (const auto* animal : animals) {
        animalManager.findMutable(animal->getId())->restoreProgress(
            std::chrono::milliseconds{0}, std::max<uint32_t>(1, animal->getAvailableProducts()));
    }
    animalManager.update(std::chrono::milliseconds{0}, space, groundItems, entityIds);
    return animals.size();
}

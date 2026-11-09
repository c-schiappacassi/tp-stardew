#include "farm_loader.h"

#include <limits>

#include "entities/entity_id_generator.h"
#include "farm/farm_manager.h"

bool FarmLoader::load(const FarmLayout& layout, FarmManager& farm, EntityIdGenerator& ids) const {
    const auto validBase = [](const Position& base) {
        return base.getX() >= 0 && base.getY() > 0 &&
            base.getX() <= std::numeric_limits<uint16_t>::max() &&
            base.getY() <= std::numeric_limits<uint16_t>::max();
    };
    if (!validBase(layout.spawn)) {
        return false;
    }
    for (const auto& base : layout.terrain.walkableBasePositions()) {
        if (!validBase(base)) {
            return false;
        }
    }
    if (!farm.setMap(layout.terrain) || !farm.setReservedPositions(layout.reservedPositions)) {
        return false;
    }
    for (const auto& base : layout.tilledSoil) {
        if (!farm.tillSoil(base)) {
            return false;
        }
    }
    for (const auto& spawn : layout.animals) {
        if (!validBase(spawn.base) || ids.getNextId() == 0 ||
            !farm.addAnimal(ids.getNextId(), spawn.base, spawn.type)) {
            return false;
        }
        ids.generate();
    }
    for (const auto& spawn : layout.crops) {
        if (!validBase(spawn.base)) {
            return false;
        }
        if (!farm.isTilled(spawn.base) && !farm.tillSoil(spawn.base)) {
            return false;
        }
        if (ids.getNextId() == 0 || !farm.plant(ids.getNextId(), spawn.base, spawn.type)) {
            return false;
        }
        if (spawn.watered) {
            farm.waterCrop(ids.getNextId());
        }
        ids.generate();
    }
    for (const auto& spawn : layout.decorations) {
        if (!validBase(spawn.base) || ids.getNextId() == 0 ||
            !farm.placeDecoration(ids.getNextId(), spawn.base, spawn.item)) {
            return false;
        }
        ids.generate();
    }
    return farm.canWalk(CollisionBox{entity_dimensions::PLAYER}.at(layout.spawn));
}

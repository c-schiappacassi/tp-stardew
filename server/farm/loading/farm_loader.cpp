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
    auto reserved = layout.reservedPositions;
    reserved.push_back(layout.spawn);
    std::vector<Position> triggers;
    if (layout.mineDoor) triggers.push_back(*layout.mineDoor);
    if (layout.townDoor) triggers.push_back(*layout.townDoor);
    triggers.insert(triggers.end(), layout.beds.begin(), layout.beds.end());
    for (std::size_t i = 0; i < triggers.size(); ++i) {
        if (!validBase(triggers[i]) ||
            !layout.terrain.isWalkable(CollisionBox{entity_dimensions::PLAYER}.at(triggers[i])) ||
            !layout.terrain.hasBasePath(layout.spawn, triggers[i])) {
            return false;
        }
        for (std::size_t j = 0; j < i; ++j) {
            if (layout.terrain.sameBaseCell(triggers[i], triggers[j])) return false;
        }
        reserved.push_back(triggers[i]);
    }
    if (layout.beds.size() > 4 || (layout.mineReturn && !layout.mineDoor) ||
        (layout.townReturn && !layout.townDoor)) return false;
    if (layout.mineReturn) reserved.push_back(*layout.mineReturn);
    if (layout.townReturn) reserved.push_back(*layout.townReturn);
    if (!farm.setMap(layout.terrain) || !farm.setReservedPositions(reserved)) {
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

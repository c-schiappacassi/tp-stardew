#include "farm_loader.h"

#include <limits>
#include <stdexcept>

#include "chests/chest_manager.h"
#include "entities/entity_id_generator.h"
#include "farm/animals/animal_manager.h"
#include "farm/crops/crop_manager.h"
#include "farm/decorations/decoration_manager.h"
#include "farm/spatial/farm_terrain.h"

bool FarmLoader::load(const FarmLayout& layout, FarmTerrain& terrain, CropManager& crops,
    AnimalManager& animals, DecorationManager& decorations, ChestManager& chests, EntityIdGenerator& ids) const {
    const auto validBase = [](const Position& base) {
        return base.getX() >= 0 && base.getY() > 0 &&
            base.getX() <= std::numeric_limits<uint16_t>::max() &&
            base.getY() <= std::numeric_limits<uint16_t>::max();
    };
    if (!terrain.initialize(layout))
        return false;
    auto& space = terrain.getSpace();
    for (const auto& base : layout.tilledSoil) {
        if (!crops.tillSoil(base, terrain)) {
            return false;
        }
    }
    for (const auto& spawn : layout.animals) {
        if (!validBase(spawn.base) || ids.getNextId() == 0 ||
            !animals.add(ids.getNextId(), spawn.base, spawn.type, space)) {
            return false;
        }
        ids.generate();
    }
    for (const auto& spawn : layout.crops) {
        if (!validBase(spawn.base)) {
            return false;
        }
        if (!crops.isTilled(spawn.base, space) && !crops.tillSoil(spawn.base, terrain)) {
            return false;
        }
        if (ids.getNextId() == 0 || !crops.plant(ids.getNextId(), spawn.base, spawn.type, space)) {
            return false;
        }
        if (spawn.watered) {
            crops.water(ids.getNextId());
        }
        ids.generate();
    }
    for (const auto& spawn : layout.decorations) {
        if (!validBase(spawn.base) || ids.getNextId() == 0 ||
            !decorations.place(ids.getNextId(), spawn.base, spawn.item, space)) {
            return false;
        }
        ids.generate();
    }
    for (const auto& spawn: layout.chests) {
        if (!validBase(spawn.base) || ids.getNextId() == 0) {
            return false;
        }
        try {
            if (!chests.place(ids.getNextId(), spawn.base, space, spawn.items))
                return false;
        } catch (const std::invalid_argument&) {
            return false;
        }
        ids.generate();
    }
    return terrain.canWalk(CollisionBox{entity_dimensions::PLAYER}.at(layout.spawn));
}

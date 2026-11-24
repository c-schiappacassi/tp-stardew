#pragma once

#include "farm_layout.h"

class FarmTerrain;
class CropManager;
class AnimalManager;
class DecorationManager;
class ChestManager;
class EntityIdGenerator;

class FarmLoader {
public:
    bool load(const FarmLayout& layout, FarmTerrain& terrain, CropManager& crops,
        AnimalManager& animals, DecorationManager& decorations, ChestManager& chests, EntityIdGenerator& ids) const;
};

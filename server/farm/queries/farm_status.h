#pragma once

#include "dtos.h"

class FarmTerrain;
class CropManager;
class AnimalManager;
class DecorationManager;
class ChestManager;
class GroundItems;

class FarmStatus {
public:
    static FarmData build(const FarmTerrain& terrain, const CropManager& crops, const AnimalManager& animals, const DecorationManager& decorations,
        const ChestManager& chests, const GroundItems& groundItems);
};

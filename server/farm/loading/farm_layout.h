#pragma once

#include <vector>
#include "protocol_types.h"
#include "world/zone_map.h"

struct FarmAnimalSpawn {
    Position base;
    AnimalType type;
};

struct FarmCropSpawn {
    Position base;
    CropType type;
    bool watered = false;
};

struct FarmDecorationSpawn {
    Position base;
    ItemId item;
};

struct FarmLayout {
    ZoneMap terrain;
    Position spawn{0, 0};
    std::vector<Position> reservedPositions;
    std::vector<Position> tilledSoil;
    std::vector<FarmAnimalSpawn> animals;
    std::vector<FarmCropSpawn> crops;
    std::vector<FarmDecorationSpawn> decorations;
};

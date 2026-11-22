#pragma once

#include <map>
#include <optional>
#include <vector>

#include "chests/loading/chest_spawn.h"
#include "world/interiors/interior_layout.h"
#include "world/zone_map.h"

#include "dtos.h"
#include "protocol_types.h"

struct FarmBackgroundTile {
    Position tile{0, 0};
    uint16_t textureId = 0;
};

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

using FarmChestSpawn = ChestSpawn;

struct FarmLayout {
    ZoneMap terrain;
    Position spawn{0, 0};
    std::vector<Position> reservedPositions;
    std::vector<Position> tilledSoil;
    std::vector<FarmAnimalSpawn> animals;
    std::vector<FarmCropSpawn> crops;
    std::vector<FarmDecorationSpawn> decorations;
    std::vector<FarmChestSpawn> chests;
    std::vector<FarmBackgroundTile> background;
    std::optional<Position> mineDoor;
    std::optional<Position> mineReturn;
    std::optional<Position> townDoor;
    std::optional<Position> townReturn;
    std::vector<InteriorLayout> interiors;
    uint8_t playerHouses = 4;
};

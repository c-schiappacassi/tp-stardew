#pragma once

#include <optional>
#include <vector>
#include "protocol_types.h"
#include "world/zone_map.h"
#include "world/interiors/interior_layout.h"

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

struct FarmLayout {
    ZoneMap terrain;
    Position spawn{0, 0};
    std::vector<Position> reservedPositions;
    std::vector<Position> tilledSoil;
    std::vector<FarmAnimalSpawn> animals;
    std::vector<FarmCropSpawn> crops;
    std::vector<FarmDecorationSpawn> decorations;
    std::vector<FarmBackgroundTile> background;
    std::optional<Position> mineDoor;
    std::optional<Position> mineReturn;
    std::optional<Position> townDoor;
    std::vector<Position> extraTownDoors;
    std::optional<Position> townReturn;
    std::vector<InteriorLayout> interiors;
    uint8_t playerHouses = 4;
};

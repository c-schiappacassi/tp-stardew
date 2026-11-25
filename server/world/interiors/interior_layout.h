#pragma once

#include <map>

#include "chests/loading/chest_spawn.h"
#include "world/world_location.h"
#include "world/zone_map.h"

struct InteriorLayout {
    InteriorLocation location;
    ZoneMap terrain;
    WorldLocation outside;
    std::vector<Position> extraOutside;
    Position returnPosition{0, 0};
    Position entry{0, 0};
    Position exit{0, 0};
    std::vector<Position> beds;
    BackgroundData background;
    std::vector<ChestSpawn> chests;
};

struct InteriorTemplate {
    ZoneMap terrain;
    Position entry{0, 0};
    Position exit{0, 0};
    std::vector<Position> beds;
    BackgroundData background;

    void applyTo(InteriorLayout& room) const;
};

using InteriorTemplates = std::map<InteriorId, InteriorTemplate>;

InteriorLayout makePlayerHouseInterior(uint8_t houseIndex);
InteriorLayout makeTownInterior(InteriorId interiorId);

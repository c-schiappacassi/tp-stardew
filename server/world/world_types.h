#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "chests/loading/chest_spawn.h"
#include "dtos.h"
#include "position.h"
#include "protocol_types.h"
#include "world/zone_map.h"

// Location & Portals
struct WorldLocation {
    Zone zone = Zone::Farm;
    uint8_t level = 0;
    std::optional<InteriorLocation> interior;
    Position position{0, 0};
};

struct ZonePortal {
    WorldLocation source;
    WorldLocation destination;
};

// Interiors
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

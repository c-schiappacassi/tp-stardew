#include "world_queries.h"

#include "common/map/map_rules.h"
#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "player/queries/player_snapshot.h"
#include "town/town_map.h"
#include "world/interiors/interior_rooms.h"
#include "world/travel/world_travel.h"

WorldQueries::WorldQueries(const DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorRooms& interiors, const WorldTravel& travel):
    dungeon(dungeon), farm(farm), town(town), interiors(interiors), travel(travel) {}

ConstMapData WorldQueries::buildConstMap() const {
    ConstMapData map;

    map.farm.floor = farm.backgroundData();

    if (const auto entrance = travel.getDungeonEntrance(Zone::Farm)) {
        const int rowsBelow = map_rules::doorRowsBelowSprite(PortalId::FarmToMine);
        map.farm.portals.push_back({Position(entrance->getX(), entrance->getY() - rowsBelow * 16), PortalId::FarmToMine});
    }
    map.levels.resize(DUNGEON_ROOM_COUNT);
    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        map.levels[number - 1].portals = dungeon.getPortals(number, false);
    }
    const int townTile = town.getTileSize();
    map.town.floor.reserve(static_cast<size_t>(town.getWidth()) * town.getHeight());
    for (int y = 0; y < town.getHeight(); ++y) {
        for (int x = 0; x < town.getWidth(); ++x) {
            map.town.floor.push_back({Position(x * townTile, y * townTile), TileTextureId::Grass});
        }
    }
    interiors.completeMap(map);
    return map;
}

bool WorldQueries::canMove(const PlayerSnapshot& player, const Position& next) const {
    const auto bounds = player.getCollisionBounds(next);
    switch (player.getZone()) {
        case Zone::Dungeon:
            return dungeon.canWalk(player.getDungeonLevel(), bounds);
        case Zone::Farm:
            return farm.isInitialized() && farm.canWalk(bounds);
        case Zone::InsideHouse:
            return interiors.canMove(player, next);
        case Zone::Town:
            return town.canWalk(bounds);
        default:
            return false;
    }
}

void WorldQueries::completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const {
    data.combatProgress = dungeon.getCombatProgress();
    data.zoneData = std::monostate{};
    data.openChest.reset();
    if (player.getZone() == Zone::Farm) {
        data.zoneData = farm.getPublicData();
        data.openChest = farm.openChestFor(player);
        return;
    }
    if (player.getZone() == Zone::InsideHouse && player.getInterior()) {
        data.zoneData = interiors.dataFor(*player.getInterior());
        data.openChest = interiors.openChestFor(player);
        return;
    }
    if (player.getZone() == Zone::Dungeon) {
        const auto mine = dungeon.getMineData(player.getDungeonLevel(), includeBroken);
        if (mine) {
            data.zoneData = *mine;
            data.openChest = dungeon.openChestFor(player);
        }
    }
}

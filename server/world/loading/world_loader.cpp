#include "world_loader.h"

#include <algorithm>
#include <utility>

#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "farm/loading/farm_layout.h"
#include "town/town_map.h"
#include "world/interiors/interior_rooms.h"
#include "world/travel/world_travel.h"

WorldLoader::WorldLoader(DungeonManager& dungeon, FarmManager& farm, TownMap& town, InteriorRooms& interiors, EntityIdGenerator& ids, WorldTravel& travel, uint32_t seed):
    dungeon(dungeon), farm(farm), town(town), interiors(interiors), entityIds(ids), travel(travel), farmSeed(seed) {}

bool WorldLoader::loadFarm(uint8_t worldId, const FarmLayout& layout) {
    if (loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    FarmManager preparedFarm{farm.getConfig(), farmSeed};
    auto preparedIds = entityIds;
    InteriorRooms preparedInteriors;
    if (!preparedFarm.initialize(layout, preparedIds) || !preparedInteriors.initialize(layout.interiors, preparedIds)) {
        return false;
    }
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (const auto& room: layout.interiors) {
        if (room.location.interior_id == InteriorId::PlayerHouse && room.location.house_index >= layout.playerHouses)
            return false;
        if (room.outside.zone == Zone::Farm && (!preparedFarm.canWalk(playerBox.at(room.outside.position)) || !preparedFarm.canWalk(playerBox.at(room.returnPosition))))
            return false;
    }
    farm = std::move(preparedFarm);
    interiors = std::move(preparedInteriors);
    entityIds = preparedIds;
    loadedWorld = worldId;
    travel.configureFarm(layout);
    town = TownMap{};
    return true;
}

bool WorldLoader::loadDungeon(uint8_t worldId, const std::map<uint8_t, DungeonRoomLayout>& layout) {
    if (farm.isInitialized() && loadedWorld && *loadedWorld != worldId) {
        return false;
    }
    if (!dungeon.initialize(layout, entityIds)) {
        return false;
    }
    loadedWorld = worldId;
    travel.resetDungeon();
    return true;
}

bool WorldLoader::validTownDoor(const TownMap& candidate, const Position& door, const Position& returnPosition, const std::vector<Position>& registered) const {
    if (!candidate.canWalk(CollisionBox{entity_dimensions::PLAYER}.at(door)) || !candidate.getMap().hasBasePath(door, returnPosition))
        return false;
    for (const auto destination: {Zone::Farm, Zone::Dungeon}) {
        const auto connection = candidate.connectionTo(destination);
        if (connection && candidate.getMap().sameBaseCell(*connection, door))
            return false;
    }
    return std::none_of(registered.begin(), registered.end(), [&](const auto& other) { return candidate.getMap().sameBaseCell(other, door); });
}

bool WorldLoader::loadTown(const TownConfig& config) {
    TownMap prepared;
    if (!loadedWorld || !prepared.initialize(config) || !travel.canConnectTown(prepared))
        return false;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    std::vector<Position> doors;
    for (const auto& room: interiors) {
        if (room.getOutside().zone != Zone::Town)
            continue;
        if (!prepared.canWalk(playerBox.at(room.getReturnPosition())) || !validTownDoor(prepared, room.getOutside().position, room.getReturnPosition(), doors))
            return false;
        doors.push_back(room.getOutside().position);
        for (const auto& door: room.getExtraOutside()) {
            if (!validTownDoor(prepared, door, room.getReturnPosition(), doors))
                return false;
            doors.push_back(door);
        }
    }
    town = std::move(prepared);
    return true;
}

bool WorldLoader::configureDungeonEntrance(const DungeonEntrance& entrance) { return loadedWorld && travel.configureEntrance(entrance); }

bool WorldLoader::acceptsWorld(uint8_t worldId) const { return !loadedWorld || *loadedWorld == worldId; }

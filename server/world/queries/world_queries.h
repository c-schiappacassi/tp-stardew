#pragma once

#include "dtos.h"

class DungeonManager;
class FarmManager;
class TownMap;
class InteriorRooms;
class WorldTravel;
class PlayerSnapshot;

class WorldQueries {
private:
    const DungeonManager& dungeon;
    const FarmManager& farm;
    const TownMap& town;
    const InteriorRooms& interiors;
    const WorldTravel& travel;

public:
    WorldQueries(const DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorRooms& interiors, const WorldTravel& travel);
    ConstMapData buildConstMap() const;
    bool canMove(const PlayerSnapshot& player, const Position& next) const;
    void completeZoneData(const PlayerSnapshot& player, PlayerDataLocal& data, bool includeBroken) const;
};

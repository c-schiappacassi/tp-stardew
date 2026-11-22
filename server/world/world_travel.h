#pragma once

#include <cstdint>
#include <map>
#include <optional>

#include "position.h"
#include "protocol_types.h"
#include "world_location.h"

class DungeonManager;
class FarmManager;
class Player;
class TownMap;
class InteriorManager;
struct FarmLayout;
struct DungeonEntrance;

class WorldTravel {
private:
    struct ReturnPoint {
        Zone zone;
        Position position;
    };

    struct Entrance {
        Zone zone;
        Position position;
        Position returnPosition;
    };

    struct FarmTownPassage {
        Position door;
        Position returnPosition;
    };

    std::optional<Entrance> entrance;
    std::optional<FarmTownPassage> farmTown;
    std::map<uint16_t, ReturnPoint> mineReturns;
    std::vector<ZonePortal> portals;

    bool canArrive(const Player& player, const ReturnPoint& target,const FarmManager& farm, const TownMap& town) const;
    bool enterMine(Player& player, uint16_t playerId, const ReturnPoint& origin, DungeonManager& dungeon);

public:
    void configureFarm(const FarmLayout& layout);
    void resetDungeon();
    bool configureEntrance(const DungeonEntrance& data, const FarmManager& farm, const TownMap& town);
    std::optional<Position> getDungeonEntrance(Zone zone) const;
    bool canConnectTown(const TownMap& town, const FarmManager& farm) const;
    bool use(Player& player, uint16_t playerId, DungeonManager& dungeon,const FarmManager& farm, const TownMap& town, const InteriorManager& interiors);
    void finishDay();
};

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "position.h"
#include "protocol_types.h"
#include "world_location.h"

class DungeonManager;
class FarmManager;
class PlayerManager;
class PlayerSnapshot;
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
        std::vector<Position> extraDoors;
    };

    std::optional<Entrance> entrance;
    std::optional<FarmTownPassage> farmTown;
    std::map<uint16_t, ReturnPoint> mineReturns;
    std::vector<ZonePortal> portals;

    bool canArrive(const PlayerSnapshot& player, const ReturnPoint& target, const FarmManager& farm, const TownMap& town) const;
    bool enterMine(PlayerManager& players, uint16_t playerId, const ReturnPoint& origin, DungeonManager& dungeon);

public:
    void configureFarm(const FarmLayout& layout);
    void resetDungeon();
    bool configureEntrance(const DungeonEntrance& data, const FarmManager& farm, const TownMap& town);
    std::optional<Position> getDungeonEntrance(Zone zone) const;
    bool canConnectTown(const TownMap& town, const FarmManager& farm) const;
    bool use(PlayerManager& players, uint16_t playerId, DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorManager& interiors);
    void finishDay();
    void onTeleport(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& destination, const FarmManager& farm);
};

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "world/world_location.h"

class DungeonManager;
class FarmManager;
class PlayerManager;
class PlayerSnapshot;
class TownMap;
class InteriorRooms;
class EntityIdGenerator;
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

    DungeonManager& dungeon;
    const FarmManager& farm;
    const TownMap& town;
    const InteriorRooms& interiors;
    EntityIdGenerator& ids;
    std::optional<Entrance> entrance;
    std::optional<FarmTownPassage> farmTown;
    std::map<uint16_t, ReturnPoint> mineReturns;
    std::vector<ZonePortal> portals;

    bool canArrive(const PlayerSnapshot& player, const ReturnPoint& target) const;
    bool sameBaseCell(const PlayerSnapshot& player, const WorldLocation& location) const;
    bool enterMine(PlayerManager& players, uint16_t playerId, const ReturnPoint& origin);
    std::optional<bool> usePortal(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player);
    std::optional<bool> useTownPassage(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player);
    bool useMinePassage(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player);
    bool useEntrance(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player);
    bool teleportTo(PlayerManager& players, uint16_t playerId, const PlayerSnapshot& player, Zone destination, uint8_t level, std::optional<InteriorLocation> inside);
    void onTeleport(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& destination);
    void notifyTravel(uint16_t playerId, const PlayerSnapshot& origin, const PlayerSnapshot& arrived);

public:
    WorldTravel(DungeonManager& dungeon, const FarmManager& farm, const TownMap& town, const InteriorRooms& interiors, EntityIdGenerator& ids);
    void configureFarm(const FarmLayout& layout);
    void resetDungeon();
    bool configureEntrance(const DungeonEntrance& data);
    std::optional<Position> getDungeonEntrance(Zone zone) const;
    bool canConnectTown(const TownMap& candidate) const;
    bool use(PlayerManager& players, uint16_t playerId);
    bool teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level, std::optional<InteriorLocation> inside);
    void finishDay();
};

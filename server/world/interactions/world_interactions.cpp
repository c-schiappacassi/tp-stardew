#include "world_interactions.h"

#include "dungeon/dungeon_manager.h"
#include "farm/farm_manager.h"
#include "player/player_manager.h"
#include "world/interiors/interior_rooms.h"

WorldInteractions::WorldInteractions(DungeonManager& dungeon, FarmManager& farm, InteriorRooms& interiors, EntityIdGenerator& ids):
    dungeon(dungeon), farm(farm), interiors(interiors), entityIds(ids) {}

void WorldInteractions::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now) {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return;
    if (player.getZone() == Zone::Dungeon) {
        dungeon.useSelectedItem(players, playerId, direction, now, entityIds);
    } else if (player.getZone() == Zone::Farm && farm.isInitialized()) {
        farm.useSelectedItem(players, playerId, direction, entityIds, now);
    } else if (player.getZone() == Zone::InsideHouse) {
        interiors.useSelectedItem(players, playerId, direction, entityIds, now);
    }
}

void WorldInteractions::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    switch (players.snapshot(playerId).getZone()) {
        case Zone::Farm:
            farm.depositInChest(players, playerId, command);
            break;
        case Zone::Dungeon:
            dungeon.depositInChest(players, playerId, command);
            break;
        case Zone::InsideHouse:
            interiors.depositInChest(players, playerId, command);
            break;
        default:
            break;
    }
}

void WorldInteractions::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    switch (players.snapshot(playerId).getZone()) {
        case Zone::Farm:
            farm.withdrawFromChest(players, playerId, command);
            break;
        case Zone::Dungeon:
            dungeon.withdrawFromChest(players, playerId, command);
            break;
        case Zone::InsideHouse:
            interiors.withdrawFromChest(players, playerId, command);
            break;
        default:
            break;
    }
}

void WorldInteractions::collectNearbyItems(PlayerManager& players, const std::vector<uint16_t>& playerIds) {
    for (const auto id: playerIds) {
        const auto zone = players.snapshot(id).getZone();
        if (zone == Zone::Farm)
            farm.collectNearbyItems(players, id);
        else if (zone == Zone::Dungeon)
            dungeon.collectNearbyItems(players, id);
        else if (zone == Zone::InsideHouse)
            interiors.collectNearbyItems(players, id);
    }
}

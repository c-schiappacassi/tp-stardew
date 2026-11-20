#include "world_manager.h"

#include "player/player.h"

bool WorldManager::teleport(Player& player, uint16_t playerId, Zone destination, uint8_t level,std::optional<InteriorLocation> inside) {
    if (!player.canAct()) return false;
    const Player origin = player;
    const bool wasInMine = player.getZone() == Zone::Dungeon;
    bool moved = false;
    if (destination == Zone::Dungeon) {
        moved = dungeon.teleportToRoom(player, level);
    } else if (destination == Zone::Farm) {
        moved = farm.spawnPlayer(player);
    } else if (destination == Zone::Town) {
        const auto entry = town.connectionTo(Zone::Farm);
        if (entry && town.canWalk(player.getCollisionBounds(*entry))) {
            moved = player.changeLocation(Zone::Town, 0, *entry);
        }
    } else if (destination == Zone::InsideHouse && inside) {
        const auto* room = interiors.find(*inside);
        if (room && interiors.canWalk(*inside, player.getCollisionBounds(room->entry))) {
            moved = player.changeLocation(Zone::InsideHouse, 0, room->entry, inside);
        }
    }
    if (moved) {
        travel.onTeleport(playerId, origin, player, farm);
        if (wasInMine || destination == Zone::Dungeon) {
            dungeon.onTravel(playerId, player.getDungeonLevel(), player.canBeAttacked());
        }
    }
    return moved;
}

std::size_t WorldManager::careCrops() { return farm.careCrops(); }
std::size_t WorldManager::careAnimals() { return farm.careAnimals(entityIds); }

#include "player/player_manager.h"

#include "world_manager.h"

bool WorldManager::teleport(PlayerManager& players, uint16_t playerId, Zone destination, uint8_t level, std::optional<InteriorLocation> inside) {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return false;
    const auto origin = player;
    const bool wasInMine = player.getZone() == Zone::Dungeon;
    bool moved = false;
    if (destination == Zone::Dungeon) {
        moved = dungeon.teleportToRoom(players, playerId, level);
    } else if (destination == Zone::Farm) {
        const auto spawn = farm.spawnFor(player);
        moved = spawn && players.apply(playerId, player_action::RestoreLocation{spawn->zone, spawn->level, spawn->position, spawn->interior}).applied;
    } else if (destination == Zone::Town) {
        const auto entry = town.connectionTo(Zone::Farm);
        if (entry && town.canWalk(player.getCollisionBounds(*entry))) {
            moved = players.apply(playerId, player_action::ChangeLocation{Zone::Town, 0, *entry}).applied;
        }
    } else if (destination == Zone::InsideHouse && inside) {
        const auto* room = interiors.find(*inside);
        if (room && interiors.canWalk(*inside, player.getCollisionBounds(room->entry))) {
            moved = players.apply(playerId, player_action::ChangeLocation{Zone::InsideHouse, 0, room->entry, inside}).applied;
        }
    }
    if (moved) {
        const auto arrived = players.snapshot(playerId);
        travel.onTeleport(playerId, origin, arrived, farm);
        if (wasInMine || destination == Zone::Dungeon) {
            dungeon.onTravel(playerId, arrived.getDungeonLevel(), arrived.canBeAttacked());
        }
    }
    return moved;
}

std::size_t WorldManager::careCrops() { return farm.careCrops(); }
std::size_t WorldManager::careAnimals() { return farm.careAnimals(entityIds); }

bool WorldManager::spawnEnemies(const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, const std::vector<Position>& occupied) {
    return dungeon.spawnEnemies(players, playerId, type, quantity, pairs, entityIds, occupied);
}

uint16_t WorldManager::killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly) { return dungeon.killEnemies(players, playerId, nearestOnly, entityIds); }

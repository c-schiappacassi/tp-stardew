#include "dungeon_interactions.h"

#include "dungeon/combat/dungeon_combat.h"
#include "dungeon/mining/dungeon_mining.h"
#include "dungeon/rooms/dungeon_rooms.h"
#include "player/player_manager.h"
#include "world/interaction_targeting.h"

DungeonInteractions::DungeonInteractions(DungeonRooms& rooms, DungeonMining& mining, MineralGenerator& generator, DungeonCombat& combat):
    rooms(rooms), mining(mining), generator(generator), combat(combat) {}

bool DungeonInteractions::completeMining(DungeonRoom& room, const MiningResult& result) const {
    if (result.droppedSilver)
        combat.silverMined(room);
    return result.acted;
}

bool DungeonInteractions::mineMineral(PlayerManager& players, uint16_t playerId, EntityId id, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) const {
    const auto player = players.snapshot(playerId);
    auto* room = rooms.find(player.getDungeonLevel());
    if (!room)
        return false;
    return completeMining(*room, mining.mine(players, playerId, *room, id, generator, ids, now));
}

bool DungeonInteractions::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now, EntityIdGenerator& ids) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !player.canAct())
        return false;
    auto* room = rooms.find(player.getDungeonLevel());
    if (!room)
        return false;
    const auto* item = player.getSelectedItem();
    if (const auto handled = room->interactWithChest(players, playerId, direction, ids, now))
        return *handled;
    if (!player.canPerformAction(now))
        return false;
    if (item && item->item_id == ItemId::Sword)
        return combat.attack(players, playerId, *room, direction, ids, now);
    return completeMining(*room, mining.useSelectedItem(players, playerId, *room, generator, ids, direction, now));
}

bool DungeonInteractions::collectNearbyItems(PlayerManager& players, uint16_t playerId) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon)
        return false;
    auto* room = rooms.find(player.getDungeonLevel());
    if (!room)
        return false;
    const bool stored = room->getStorageItems().collect(
        players, playerId, [&](const Position& position) { return InteractionTargeting{}.hasClearPath(player.getPosition(), position, room->getMap()); });
    const bool collected = room->getGroundItems().collect(players, playerId, [&](const Position& position) {
        return room->hasClearLine(player.getCollisionCenter(), Position{position.getX(), position.getY() - 1});
    });
    return collected || stored;
}

bool DungeonInteractions::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) const {
    const auto player = players.snapshot(playerId);
    auto* room = player.getZone() == Zone::Dungeon ? rooms.find(player.getDungeonLevel()) : nullptr;
    return room && room->getChests().deposit(players, playerId, room->getMap(), command);
}

bool DungeonInteractions::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) const {
    const auto player = players.snapshot(playerId);
    auto* room = player.getZone() == Zone::Dungeon ? rooms.find(player.getDungeonLevel()) : nullptr;
    return room && room->getChests().withdraw(players, playerId, room->getMap(), command);
}

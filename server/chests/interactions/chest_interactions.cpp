#include "chest_interactions.h"

#include <limits>

#include "chests/chest_manager.h"
#include "entities/entity_id_generator.h"
#include "items/ground_items.h"
#include "player/player_manager.h"
#include "world/zone_map.h"

bool ChestInteractions::supportsChests(const PlayerSnapshot& player) {
    return player.getZone() == Zone::Farm || player.getZone() == Zone::Dungeon ||
        (player.getZone() == Zone::InsideHouse && player.getInterior() && player.getInterior()->interior_id == InteriorId::PlayerHouse);
}

bool ChestInteractions::overlapsPlayer(const PlayerManager& players, const PlayerSnapshot& player, const CollisionBounds& bounds) {
    for (const auto id: players.connectedPlayerIds()) {
        const auto other = players.snapshot(id);
        if (other.getZone() == player.getZone() && other.getDungeonLevel() == player.getDungeonLevel() && other.getInterior() == player.getInterior() &&
            other.getCollisionBounds(other.getPosition()).intersects(bounds))
            return true;
    }
    return false;
}

bool ChestInteractions::canAccess(const PlayerSnapshot& player, const ChestSnapshot& chest, const ZoneMap& terrain) const {
    return supportsChests(player) && player.canAct() && player.canReach(chest.getPosition(), INTERACTION_RANGE) &&
        terrain.hasClearPath(player.getPosition(), chest.getPosition());
}

std::optional<ChestSnapshot> ChestInteractions::findTarget(const PlayerSnapshot& player, const ChestManager& chests, const ZoneMap& terrain, Direction direction) const {
    const Chest* closest = nullptr;
    int64_t closestDistance = std::numeric_limits<int64_t>::max();
    for (const auto& [id, chest]: chests.getChests()) {
        if (!terrain.canReach(player.getPosition(), chest.getPosition(), direction, INTERACTION_RANGE))
            continue;
        const int64_t dx = static_cast<int64_t>(player.getPosition().getX()) - chest.getPosition().getX();
        const int64_t dy = static_cast<int64_t>(player.getPosition().getY()) - chest.getPosition().getY();
        if (dx * dx + dy * dy < closestDistance) {
            closest = &chest;
            closestDistance = dx * dx + dy * dy;
        }
    }
    return closest ? std::optional<ChestSnapshot>{closest->snapshot()} : std::nullopt;
}

std::optional<bool> ChestInteractions::interact(PlayerManager& players, uint16_t playerId, ChestManager& chests, Placement& placement, const ZoneMap& terrain,
    GroundItems& groundItems, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) const {
    const auto player = players.snapshot(playerId);
    if (!supportsChests(player) || !player.canAct())
        return std::nullopt;
    const auto* selected = player.getSelectedItem();
    const bool shovel = selected && selected->item_id == ItemId::Shovel;
    const auto target = findTarget(player, chests, terrain, direction);
    if (target && !shovel) {
        if (!canAccess(player, *target, terrain))
            return false;
        if (player.getOpenChestId() == target->getId())
            return players.apply(playerId, player_action::CloseChest{}).applied;
        players.apply(playerId, player_action::FaceTarget{target->getPosition()});
        return players.apply(playerId, player_action::OpenChest{target->getId()}).applied;
    }
    if (target && shovel) {
        const auto adjacent = terrain.findAdjacentPosition(player.getPosition(), direction,
            player.getCollisionBounds(player.getPosition()), CollisionBox{entity_dimensions::DECORATION});
        if (!adjacent || !terrain.sameBaseCell(*adjacent, target->getPosition()))
            return std::nullopt;
        if (!target->isEmpty() || !player.canUseTool(now) || !groundItems.drop(target->getPosition(), ItemId::Chest, 1, Quality::None, ids))
            return false;
        if (!chests.removeEmpty(target->getId(), placement))
            return false;
        players.apply(playerId, player_action::CloseChest{});
        for (const auto otherId: players.connectedPlayerIds()) {
            if (players.snapshot(otherId).getOpenChestId() == target->getId())
                players.apply(otherId, player_action::CloseChest{});
        }
        players.apply(playerId, player_action::FaceDirection{direction});
        return players.apply(playerId, player_action::UseTool{now}).applied;
    }
    if (!selected || selected->item_id != ItemId::Chest)
        return std::nullopt;
    if (!player.canPerformAction(now) || ids.getNextId() == 0)
        return false;
    const auto position = terrain.findAdjacentPosition(player.getPosition(), direction,
        player.getCollisionBounds(player.getPosition()), CollisionBox{entity_dimensions::DECORATION});
    if (!position || overlapsPlayer(players, player, CollisionBox{entity_dimensions::DECORATION}.at(*position)))
        return false;
    const auto id = ids.getNextId();
    if (!chests.place(id, *position, placement))
        return false;
    if (!players.apply(playerId, player_action::RemoveItem{player.getSelectedSlot(), 1}).applied) {
        chests.removeEmpty(id, placement);
        return false;
    }
    ids.generate();
    players.apply(playerId, player_action::CloseChest{});
    players.apply(playerId, player_action::FaceDirection{direction});
    return players.apply(playerId, player_action::PerformAction{ACTION_ENERGY, ACTION_INTERVAL, now}).applied;
}

bool ChestInteractions::deposit(PlayerManager& players, uint16_t playerId, ChestManager& chests, const ZoneMap& terrain, const ItemTransferDTO& command) const {
    const auto player = players.snapshot(playerId);
    const auto chest = chests.snapshot(command.chestId);
    return chest && player.getOpenChestId() == command.chestId && canAccess(player, *chest, terrain) &&
        chests.apply(command.chestId, players, playerId, chest_action::Deposit{command.slot, command.quantity});
}

bool ChestInteractions::withdraw(PlayerManager& players, uint16_t playerId, ChestManager& chests, const ZoneMap& terrain, const SelectItemDTO& command) const {
    const auto player = players.snapshot(playerId);
    const auto chest = chests.snapshot(command.chestId);
    return chest && player.getOpenChestId() == command.chestId && canAccess(player, *chest, terrain) &&
        chests.apply(command.chestId, players, playerId, chest_action::Withdraw{command.slot, command.quantity});
}

std::optional<OpenChestData> ChestInteractions::contentsFor(const PlayerSnapshot& player, const ChestManager& chests, const ZoneMap& terrain) const {
    const auto id = player.getOpenChestId();
    const auto chest = id ? chests.snapshot(*id) : std::nullopt;
    return chest && canAccess(player, *chest, terrain) ? std::optional<OpenChestData>{chest->getContents()} : std::nullopt;
}

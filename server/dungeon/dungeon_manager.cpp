#include "dungeon_manager.h"

#include "player/player_manager.h"

DungeonManager::DungeonManager(const DungeonConfig& config, uint32_t seed):
        generator(config, seed), placementRandom(seed), mining(config.miningRange), loader(config), combat(config.enemies, seed) {
    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        rooms.try_emplace(number, number);
    }
}

DungeonRoom* DungeonManager::findRoom(uint8_t number) {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

const DungeonRoom* DungeonManager::findRoom(uint8_t number) const {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

bool DungeonManager::initialize(const std::map<uint8_t, DungeonRoomLayout>& layout, EntityIdGenerator& entityIds) {
    auto prepared = *this;
    auto preparedIds = entityIds;
    auto preparedLayouts = layout;
    for (auto& [number, roomLayout]: preparedLayouts) {
        for (const auto& trap: roomLayout.traps) {
            roomLayout.reservedPositions.push_back(trap.pos);
        }
    }
    if (!prepared.loader.load(preparedLayouts, prepared.rooms, prepared.generator, preparedIds, prepared.placementRandom) ||
        !prepared.combat.initialize(prepared.rooms, preparedLayouts, preparedIds)) {
        return false;
    }
    *this = std::move(prepared);
    entityIds = preparedIds;
    return true;
}

std::optional<WorldLocation> DungeonManager::spawnFor(const PlayerSnapshot& player) const { return travel.spawnFor(player, rooms); }

bool DungeonManager::canWalk(uint8_t number, const CollisionBounds& bounds) const {
    const auto* room = findRoom(number);
    return room && room->canWalk(bounds);
}

std::optional<MineData> DungeonManager::getMineData(uint8_t number, bool includeBroken) const {
    const auto* room = findRoom(number);
    auto mine = room ? room->getMineData(includeBroken) : std::nullopt;
    if (mine) {
        mine->current_level_data.combat.visitId = combat.getVisit();
    }
    return mine;
}

std::vector<PortalData> DungeonManager::getPortals(uint8_t number) const {
    const auto* room = findRoom(number);
    if (!room || !room->getPassages()) {
        return {};
    }
    const auto& passages = *room->getPassages();
    std::vector<PortalData> portals{{passages.exit, PortalId::MineToFarm}};
    if (passages.nextRoom) {
        portals.push_back({*passages.nextRoom, PortalId::ToNextLevel});
    }
    return portals;
}

std::optional<DungeonCombatData> DungeonManager::getCombatData(uint8_t number) const {
    const auto* room = findRoom(number);
    if (!room) {
        return std::nullopt;
    }
    auto data = room->getCombatData();
    data.visitId = combat.getVisit();
    return data;
}

bool DungeonManager::mineMineral(PlayerManager& players, uint16_t playerId, EntityId id, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    const auto player = players.snapshot(playerId);
    auto* room = findRoom(player.getDungeonLevel());
    if (!room) {
        return false;
    }
    const auto result = mining.mine(players, playerId, *room, id, generator, ids, now);
    if (result.droppedSilver) {
        combat.silverMined(*room);
    }
    return result.acted;
}

bool DungeonManager::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, std::chrono::steady_clock::time_point now, EntityIdGenerator& ids) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !player.canPerformAction(now)) {
        return false;
    }
    auto* room = findRoom(player.getDungeonLevel());
    if (!room) {
        return false;
    }
    const auto* item = player.getSelectedItem();
    if (item && item->item_id == ItemId::Sword) {
        return combat.attack(players, playerId, *room, direction, ids, now);
    }
    const auto result = mining.useSelectedItem(players, playerId, *room, generator, ids, direction, now);
    if (result.droppedSilver) {
        combat.silverMined(*room);
    }
    return result.acted;
}

std::vector<LootData> DungeonManager::getGroundItems(uint8_t number) const {
    const auto* room = findRoom(number);
    return room ? room->getGroundItems().getPublicData() : std::vector<LootData>{};
}

bool DungeonManager::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon) {
        return false;
    }
    auto* room = findRoom(player.getDungeonLevel());
    return room && room->getGroundItems().collect(players, playerId, [&](const Position& position) {
        return room->hasClearLine(player.getCollisionCenter(), Position{position.getX(), position.getY() - 1});
    });
}

void DungeonManager::update(
    std::chrono::milliseconds elapsed, const std::vector<Combatant>& combatants, PlayerManager& players, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    combat.update(elapsed, rooms, combatants, players, ids, now);
}

void DungeonManager::onTravel(uint16_t playerId, uint8_t room, bool survived) { combat.onTravel(playerId, room, survived, rooms); }

const CombatProgressData& DungeonManager::getCombatProgress() const { return combat.getProgress(); }
bool DungeonManager::combatCompleted() const { return combat.completed(); }

bool DungeonManager::enter(PlayerManager& players, uint16_t playerId) { return travel.enter(players, playerId, rooms); }

bool DungeonManager::advanceRoom(PlayerManager& players, uint16_t playerId) { return travel.advanceRoom(players, playerId, rooms); }

bool DungeonManager::teleportToRoom(PlayerManager& players, uint16_t playerId, uint8_t number) const {
    const auto player = players.snapshot(playerId);
    const auto* room = findRoom(number);
    if (!room || !room->getPassages() || !room->canWalk(player.getCollisionBounds(room->getPassages()->entry)))
        return false;
    return players.apply(playerId, player_action::ChangeLocation{Zone::Dungeon, number, room->getPassages()->entry}).applied;
}

bool DungeonManager::canExit(const PlayerSnapshot& player) const { return travel.canExit(player, rooms); }

bool DungeonManager::spawnEnemies(
    const PlayerManager& players, uint16_t playerId, EnemyType type, uint16_t quantity, bool pairs, EntityIdGenerator& ids, const std::vector<Position>& occupied) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !player.canAct() || !combat.prepareVisit(players, playerId, rooms, ids))
        return false;
    auto* room = findRoom(player.getDungeonLevel());
    if (!room)
        return false;
    auto reserved = occupied;
    reserved.push_back(player.getPosition());
    return combat.spawnEnemies(*room, type, quantity, pairs, ids, reserved);
}

uint16_t DungeonManager::killEnemies(PlayerManager& players, uint16_t playerId, bool nearestOnly, EntityIdGenerator& ids) {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::Dungeon || !combat.prepareVisit(players, playerId, rooms, ids))
        return 0;
    auto* room = findRoom(player.getDungeonLevel());
    if (!room)
        return 0;
    return combat.killEnemies(players, playerId, *room, nearestOnly, ids);
}

void DungeonManager::removeBrokenMinerals() {
    for (auto& [number, room]: rooms) {
        room.removeBrokenMinerals();
    }
}

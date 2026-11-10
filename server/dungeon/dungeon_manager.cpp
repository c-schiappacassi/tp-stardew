#include "dungeon_manager.h"

#include "player/player.h"

DungeonManager::DungeonManager(const DungeonConfig& config, uint32_t seed):
        generator(config, seed),
        placementRandom(seed),
        mining(config.miningRange),
        loader(config),
        combat(config.enemies, seed) {
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

bool DungeonManager::initialize(const std::map<uint8_t, DungeonRoomLayout>& layout,EntityIdGenerator& entityIds) {
    auto prepared = *this;
    auto preparedIds = entityIds;
    auto preparedLayouts = layout;
    for (auto& [number, roomLayout]: preparedLayouts) {
        for (const auto& trap: roomLayout.traps) {
            roomLayout.reservedPositions.push_back(trap.pos);
        }
    }
    if (!prepared.loader.load(preparedLayouts, prepared.rooms, prepared.generator, preparedIds,prepared.placementRandom) ||!prepared.combat.initialize(prepared.rooms, preparedLayouts, preparedIds)) {
        return false;
    }
    *this = std::move(prepared);
    entityIds = preparedIds;
    return true;
}

bool DungeonManager::spawnPlayer(Player& player) const { 
    return travel.spawnPlayer(player, rooms); 
}

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

std::optional<DungeonCombatData> DungeonManager::getCombatData(uint8_t number) const {
    const auto* room = findRoom(number);
    if (!room) {
        return std::nullopt;
    }
    auto data = room->getCombatData();
    data.visitId = combat.getVisit();
    return data;
}

bool DungeonManager::mineMineral(Player& player, EntityId id,
                                 std::chrono::steady_clock::time_point now) {
    auto* room = findRoom(player.getDungeonLevel());
    if (!room) {
        return false;
    }
    const auto result = mining.mine(player, *room, id, generator, now);
    if (result.obtainedSilver) {
        combat.silverMined(*room);
    }
    return result.acted;
}

bool DungeonManager::useSelectedItem(Player& player, Direction direction,std::chrono::steady_clock::time_point now,EntityIdGenerator& ids, uint16_t playerId) {
    auto* room = findRoom(player.getDungeonLevel());
    if (!room) {
        return false;
    }
    if (combat.collectLoot(player, *room)) {
        return true;
    }
    const auto* item = player.getSelectedItem();
    if (item && item->item_id == ItemId::Sword) {
        return combat.attack(player, playerId, *room, direction, ids, now);
    }
    const auto result = mining.useSelectedItem(player, *room, generator, direction, now);
    if (result.obtainedSilver) {
        combat.silverMined(*room);
    }
    return result.acted;
}

void DungeonManager::update(std::chrono::milliseconds elapsed,const std::vector<Combatant>& players, EntityIdGenerator& ids,std::chrono::steady_clock::time_point now) {
    combat.update(elapsed, rooms, players, ids, now);
}

void DungeonManager::onTravel(uint16_t playerId, uint8_t room, bool survived) {
    combat.onTravel(playerId, room, survived, rooms);
}

const CombatProgressData& DungeonManager::getCombatProgress() const { return combat.getProgress(); }
bool DungeonManager::combatCompleted() const { return combat.completed(); }

bool DungeonManager::enter(Player& player) { return travel.enter(player, rooms); }

bool DungeonManager::advanceRoom(Player& player) { 
    return travel.advanceRoom(player, rooms); 
}

bool DungeonManager::canExit(const Player& player) const {
     return travel.canExit(player, rooms); 
    }

void DungeonManager::removeBrokenMinerals() {
    for (auto& [number, room] : rooms) {
        room.removeBrokenMinerals();
    }
}

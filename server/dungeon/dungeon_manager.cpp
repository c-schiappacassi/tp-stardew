#include "dungeon_manager.h"
#include "player/player.h"

DungeonManager::DungeonManager(const DungeonConfig& config, uint32_t seed):generator(config, seed),placementRandom(seed),mining(config.miningRange),loader(config) {
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
    return loader.load(layout, rooms, generator, entityIds, placementRandom);
}

bool DungeonManager::canWalk(uint8_t number, const Position& pos) const {
    const auto* room = findRoom(number);
    return room && room->canWalk(pos);
}

std::optional<MineData> DungeonManager::getMineData(uint8_t number, bool includeBroken) const {
    const auto* room = findRoom(number);
    return room ? room->getMineData(includeBroken) : std::nullopt;
}

bool DungeonManager::mineMineral(Player& player,EntityId id,std::chrono::steady_clock::time_point now) {
    auto* room = findRoom(player.getDungeonLevel());
    return room && mining.mine(player, *room, id, generator, now);
}

bool DungeonManager::useSelectedItem(Player& player,Direction direction,std::chrono::steady_clock::time_point now) {
    auto* room = findRoom(player.getDungeonLevel());
    return room && mining.useSelectedItem(player, *room, generator, direction, now);
}

bool DungeonManager::enter(Player& player) {
    return travel.enter(player, rooms);
}

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
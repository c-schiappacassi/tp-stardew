#include "dungeon_rooms.h"

#include <algorithm>
#include "dungeon/dungeon_config.h"
#include "player/queries/player_snapshot.h"

DungeonRooms::DungeonRooms() {
    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        rooms.try_emplace(number, number);
        visitOrder.push_back(number);
    }
}

void DungeonRooms::shuffle(std::mt19937& random) {
    std::shuffle(visitOrder.begin(), visitOrder.end(), random);
    for (std::size_t i = 0; i < visitOrder.size(); ++i)
        at(visitOrder[i]).setVisitDepth(static_cast<uint8_t>(i + 1));
}

uint8_t DungeonRooms::firstNumber() const { return visitOrder.front(); }

std::optional<uint8_t> DungeonRooms::nextNumber(uint8_t number) const {
    const auto current = std::find(visitOrder.begin(), visitOrder.end(), number);
    return current == visitOrder.end() || std::next(current) == visitOrder.end() ? std::nullopt : std::optional<uint8_t>{*std::next(current)};
}

DungeonRoom* DungeonRooms::find(uint8_t number) {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

const DungeonRoom* DungeonRooms::find(uint8_t number) const {
    const auto it = rooms.find(number);
    return it == rooms.end() ? nullptr : &it->second;
}

DungeonRoom& DungeonRooms::at(uint8_t number) { return rooms.at(number); }
const DungeonRoom& DungeonRooms::at(uint8_t number) const { return rooms.at(number); }
std::size_t DungeonRooms::size() const { return rooms.size(); }
DungeonRooms::iterator DungeonRooms::begin() { return rooms.begin(); }
DungeonRooms::iterator DungeonRooms::end() { return rooms.end(); }
DungeonRooms::const_iterator DungeonRooms::begin() const { return rooms.begin(); }
DungeonRooms::const_iterator DungeonRooms::end() const { return rooms.end(); }

bool DungeonRooms::canWalk(uint8_t number, const CollisionBounds& bounds) const {
    const auto* room = find(number);
    return room && room->canWalk(bounds);
}

std::optional<MineData> DungeonRooms::getMineData(uint8_t number, uint32_t visit, bool includeBroken) const {
    const auto* room = find(number);
    auto data = room ? room->getMineData(includeBroken) : std::nullopt;
    if (data)
        data->current_level_data.combat.visitId = visit;
    return data;
}

std::optional<DungeonCombatData> DungeonRooms::getCombatData(uint8_t number, uint32_t visit) const {
    const auto* room = find(number);
    if (!room)
        return std::nullopt;
    auto data = room->getCombatData();
    data.visitId = visit;
    return data;
}

std::vector<PortalData> DungeonRooms::getPortals(uint8_t number, bool activeOnly) const {
    const auto* room = find(number);
    if (!room || !room->getPassages())
        return {};
    const auto& passages = *room->getPassages();
    std::vector<PortalData> portals{{passages.exit, PortalId::MineToFarm}};
    if (passages.nextRoom && (!activeOnly || nextNumber(number)))
        portals.push_back({*passages.nextRoom, PortalId::ToNextLevel});
    return portals;
}

std::vector<LootData> DungeonRooms::getGroundItems(uint8_t number) const {
    const auto* room = find(number);
    return room ? room->getGroundItems().getPublicData() : std::vector<LootData>{};
}

std::optional<OpenChestData> DungeonRooms::openChestFor(const PlayerSnapshot& player) const {
    const auto* room = player.getZone() == Zone::Dungeon ? find(player.getDungeonLevel()) : nullptr;
    return room ? room->getChests().contentsFor(player, room->getMap()) : std::nullopt;
}

void DungeonRooms::removeBrokenMinerals() {
    for (auto& [number, room]: rooms) room.removeBrokenMinerals();
}

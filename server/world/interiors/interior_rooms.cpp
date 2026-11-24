#include "interior_rooms.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

#include "entities/entity_id_generator.h"
#include "player/player_manager.h"

bool InteriorRooms::initialize(const std::vector<InteriorLayout>& layouts, EntityIdGenerator& ids) {
    std::set<std::pair<InteriorId, uint8_t>> registered;
    std::vector<Interior> prepared;
    prepared.reserve(layouts.size());
    for (const auto& layout: layouts) {
        if (!Interior::validLayout(layout) || !registered.emplace(layout.location.interior_id, layout.location.house_index).second)
            return false;
        prepared.emplace_back(layout);
    }
    for (const auto& room: prepared) {
        if (!room.hasAccessibleBeds())
            return false;
    }
    auto preparedIds = ids;
    for (std::size_t i = 0; i < prepared.size(); ++i) {
        if (!prepared[i].initializeStorage(layouts[i].chests, preparedIds))
            return false;
    }
    rooms = std::move(prepared);
    ids = preparedIds;
    return true;
}

const Interior* InteriorRooms::find(const InteriorLocation& location) const {
    const auto found = std::find_if(rooms.begin(), rooms.end(), [&](const auto& room) { return room.getLocation() == location; });
    return found == rooms.end() ? nullptr : &*found;
}

Interior* InteriorRooms::find(const InteriorLocation& location) {
    const auto found = std::find_if(rooms.begin(), rooms.end(), [&](const auto& room) { return room.getLocation() == location; });
    return found == rooms.end() ? nullptr : &*found;
}

Interior* InteriorRooms::roomFor(const PlayerSnapshot& player) {
    return player.getZone() == Zone::InsideHouse && player.getInterior() ? find(*player.getInterior()) : nullptr;
}

const Interior* InteriorRooms::roomFor(const PlayerSnapshot& player) const {
    return player.getZone() == Zone::InsideHouse && player.getInterior() ? find(*player.getInterior()) : nullptr;
}

InteriorRooms::const_iterator InteriorRooms::begin() const { return rooms.begin(); }
InteriorRooms::const_iterator InteriorRooms::end() const { return rooms.end(); }

bool InteriorRooms::canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const {
    const auto* room = find(location);
    return room && room->canWalk(bounds);
}

bool InteriorRooms::canMove(const PlayerSnapshot& player, const Position& next) const {
    const auto* room = roomFor(player);
    return room && room->canMove(player, next);
}

bool InteriorRooms::useBed(PlayerManager& players, uint16_t playerId) const {
    const auto* room = roomFor(players.snapshot(playerId));
    return room && room->useBed(players, playerId);
}

std::optional<WorldLocation> InteriorRooms::recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const {
    const Interior* house = nullptr;
    if (player.isSleeping() && player.getZone() == Zone::InsideHouse && player.getInterior() && player.getInterior()->interior_id == InteriorId::PlayerHouse)
        house = find(*player.getInterior());
    if (!house)
        house = playerId >= 1 && playerId <= 4 ? find({InteriorId::PlayerHouse, static_cast<uint8_t>(playerId - 1)}) : nullptr;
    if (!house || !house->hasBeds()) {
        const auto fallback = std::find_if(rooms.begin(), rooms.end(), [](const auto& room) {
            return room.getLocation().interior_id == InteriorId::PlayerHouse && room.hasBeds();
        });
        if (fallback == rooms.end())
            return std::nullopt;
        house = &*fallback;
    }
    return house->recoveryFor(player);
}

std::optional<NpcData> InteriorRooms::nearbyMerchant(const PlayerSnapshot& player, Direction direction) const {
    const auto* room = roomFor(player);
    return room ? room->nearbyMerchant(player, direction) : std::nullopt;
}

InteriorData InteriorRooms::dataFor(const InteriorLocation& location) const {
    const auto* room = find(location);
    return room ? room->data() : InteriorData{location.interior_id, location.house_index, {}, {}};
}

void InteriorRooms::completeMap(ConstMapData& map) const {
    for (const auto& room: rooms) {
        auto background = room.backgroundData();
        const auto& location = room.getLocation();
        switch (location.interior_id) {
            case InteriorId::PlayerHouse:
                if (map.farm_houses.size() <= location.house_index)
                    map.farm_houses.resize(location.house_index + 1);
                map.farm_houses[location.house_index] = std::move(background);
                break;
            case InteriorId::PierresShop:
                map.pierres = std::move(background);
                break;
            case InteriorId::RobinsShop:
                map.robins = std::move(background);
                break;
            case InteriorId::ClintsShop:
                map.clints = std::move(background);
                break;
            case InteriorId::MarniesShop:
                map.marnies = std::move(background);
                break;
            case InteriorId::ComunityCenter:
                map.community_center = std::move(background);
                break;
        }
    }
}

bool InteriorRooms::useSelectedItem(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    auto* room = roomFor(players.snapshot(playerId));
    return room && room->useSelectedItem(players, playerId, direction, ids, now);
}

std::optional<OpenChestData> InteriorRooms::openChestFor(const PlayerSnapshot& player) const {
    const auto* room = roomFor(player);
    return room ? room->openChestFor(player) : std::nullopt;
}

void InteriorRooms::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    auto* room = roomFor(players.snapshot(playerId));
    if (room)
        room->depositInChest(players, playerId, command);
}

void InteriorRooms::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    auto* room = roomFor(players.snapshot(playerId));
    if (room)
        room->withdrawFromChest(players, playerId, command);
}

void InteriorRooms::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    auto* room = roomFor(players.snapshot(playerId));
    if (room)
        room->collectNearbyItems(players, playerId);
}

std::vector<InteriorData> InteriorRooms::storageData() const {
    std::vector<InteriorData> result;
    for (const auto& room: rooms) {
        if (room.getLocation().interior_id == InteriorId::PlayerHouse)
            result.push_back(room.data());
    }
    // The former storage map serialized houses by index, regardless of layout order.
    std::sort(result.begin(), result.end(), [](const auto& first, const auto& second) { return first.house_index < second.house_index; });
    return result;
}

void InteriorRooms::saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const {
    for (const auto& room: rooms)
        room.saveInventories(inventories);
}

void InteriorRooms::restoreStorage(const std::vector<InteriorData>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) {
    auto prepared = *this;
    for (const auto& data: saved) {
        auto* room = prepared.find({data.interior_id, data.house_index});
        if (data.interior_id != InteriorId::PlayerHouse || !room)
            throw std::runtime_error("Invalid saved house storage");
        room->restoreStorage(data, inventories);
    }
    *this = std::move(prepared);
}

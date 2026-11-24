#include "interior_manager.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <set>
#include <stdexcept>

#include "commerce/shop_products.h"
#include "entities/collision_box.h"
#include "entities/entity_id_generator.h"
#include "player/player_manager.h"

namespace {
const CollisionBox BED_BOX{22, 32};

bool validPosition(const Position& pos) {
    return pos.getX() >= 0 && pos.getY() >= 0 && pos.getX() <= std::numeric_limits<uint16_t>::max() && pos.getY() <= std::numeric_limits<uint16_t>::max();
}

bool canApproachBed(const InteriorLayout& room, const Position& pos, const Position& bed) {
    const int dx = bed.getX() - pos.getX();
    const int dy = bed.getY() - pos.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps > 22)
        return false;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (int step = 0; step <= steps; ++step) {
        const Position point{pos.getX() + dx * step / std::max(1, steps), pos.getY() + dy * step / std::max(1, steps)};
        if (!room.terrain.isWalkable(playerBox.at(point)))
            return false;
    }
    return true;
}

ZoneMap walkingMap(const InteriorLayout& room) {
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    const auto positions = room.terrain.walkableBasePositions();
    int rows = 0;
    int columns = 0;
    for (const auto& pos: positions) {
        const auto cell = room.terrain.baseCellAt(pos).value();
        rows = std::max(rows, cell.first + 1);
        columns = std::max(columns, cell.second + 1);
    }
    std::vector<std::vector<bool>> floor(rows, std::vector<bool>(columns, false));
    for (const auto& pos: positions) {
        const auto cell = room.terrain.baseCellAt(pos).value();
        floor[cell.first][cell.second] = room.terrain.isWalkable(playerBox.at(pos)) &&
            (!room.background.interior || !room.background.interior->npc || !playerBox.at(room.background.interior->npc->pos).intersects(playerBox.at(pos))) &&
            std::none_of(room.beds.begin(), room.beds.end(), [&](const auto& bed) { return BED_BOX.at(bed).intersects(playerBox.at(pos)); });
    }
    return ZoneMap{floor, room.terrain.getCellSize()};
}
}  // namespace

bool InteriorManager::initialize(const std::vector<InteriorLayout>& layouts, EntityIdGenerator& ids) {
    std::set<std::pair<InteriorId, uint8_t>> registered;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (const auto& room: layouts) {
        if (room.location.interior_id > InteriorId::ComunityCenter || (room.location.interior_id == InteriorId::PlayerHouse && room.location.house_index >= 4) ||
            (room.location.interior_id != InteriorId::PlayerHouse && room.location.house_index != 0) ||
            !registered.emplace(room.location.interior_id, room.location.house_index).second || (room.outside.zone != Zone::Farm && room.outside.zone != Zone::Town) ||
            room.outside.level != 0 || room.outside.interior || !validPosition(room.outside.position) || !validPosition(room.returnPosition) || !validPosition(room.entry) ||
            !validPosition(room.exit) || !room.terrain.isWalkable(playerBox.at(room.entry)) || !room.terrain.isWalkable(playerBox.at(room.exit)) ||
            !room.terrain.hasBasePath(room.entry, room.exit))
            return false;
        if (room.background.interior && room.background.interior->npc) {
            const auto& npc = *room.background.interior->npc;
            const auto expected = shop_products::merchantForInterior(room.location.interior_id);
            const auto bounds = playerBox.at(npc.pos);
            if (!expected || *expected != npc.id || !validPosition(npc.pos) || !room.terrain.isWalkable(bounds) || bounds.intersects(playerBox.at(room.entry)) ||
                bounds.intersects(playerBox.at(room.exit)))
                return false;
        }
        std::vector<CollisionBounds> beds;
        for (const auto& bed: room.beds) {
            const auto bounds = BED_BOX.at(bed);
            if (!validPosition(bed) || !room.terrain.isWalkable(bounds) ||
                (room.background.interior && room.background.interior->npc && bounds.intersects(playerBox.at(room.background.interior->npc->pos))) ||
                bounds.intersects(playerBox.at(room.entry)) || bounds.intersects(playerBox.at(room.exit)) ||
                std::any_of(beds.begin(), beds.end(), [&](const auto& other) { return bounds.intersects(other); }))
                return false;
            beds.push_back(bounds);
        }
    }
    for (const auto& room: layouts) {
        const auto walking = walkingMap(room);
        if (!walking.hasBasePath(room.entry, room.exit))
            return false;
        const auto positions = walking.walkableBasePositions();
        for (const auto& bed: room.beds) {
            if (std::none_of(positions.begin(), positions.end(), [&](const auto& pos) { return walking.hasBasePath(room.entry, pos) && canApproachBed(room, pos, bed); })) {
                return false;
            }
        }
    }
    std::map<std::pair<InteriorId, uint8_t>, ChestRoom> prepared;
    auto preparedIds = ids;
    for (const auto& room: layouts) {
        if (room.location.interior_id != InteriorId::PlayerHouse) {
            if (!room.chests.empty())
                return false;
            continue;
        }
        std::vector<CollisionBounds> obstacles;
        for (const auto& bed: room.beds) obstacles.push_back(BED_BOX.at(bed));
        if (!prepared[{room.location.interior_id, room.location.house_index}].initialize(room.terrain, {room.entry, room.exit}, obstacles, room.chests, preparedIds))
            return false;
    }
    rooms = layouts;
    storage = std::move(prepared);
    ids = preparedIds;
    return true;
}

const InteriorLayout* InteriorManager::find(const InteriorLocation& location) const {
    const auto it = std::find_if(rooms.begin(), rooms.end(), [&](const auto& room) { return room.location == location; });
    return it == rooms.end() ? nullptr : &*it;
}

bool InteriorManager::canWalk(const InteriorLocation& location, const CollisionBounds& bounds) const {
    const auto* room = find(location);
    const auto chests = storage.find({location.interior_id, location.house_index});
    return room && room->terrain.isWalkable(bounds) && (chests == storage.end() || chests->second.canWalk(bounds)) &&
        (!room->background.interior || !room->background.interior->npc || !CollisionBox{entity_dimensions::PLAYER}.at(room->background.interior->npc->pos).intersects(bounds)) &&
        std::none_of(room->beds.begin(), room->beds.end(), [&](const auto& bed) { return BED_BOX.at(bed).intersects(bounds); });
}

bool InteriorManager::useBed(PlayerManager& players, uint16_t playerId) const {
    const auto player = players.snapshot(playerId);
    if (player.getZone() != Zone::InsideHouse || !player.getInterior()) {
        players.apply(playerId, player_action::LeaveBedArea{});
        return false;
    }
    const auto* room = find(*player.getInterior());
    if (!room)
        return false;
    const auto& pos = player.getPosition();
    for (const auto& bed: room->beds) {
        if (canApproachBed(*room, pos, bed))
            return player.canUseBed() && players.apply(playerId, player_action::Sleep{bed}).applied;
    }
    players.apply(playerId, player_action::LeaveBedArea{});
    return false;
}

std::optional<WorldLocation> InteriorManager::recoveryFor(const PlayerSnapshot& player, uint16_t playerId) const {
    const InteriorLayout* house = nullptr;
    if (player.isSleeping() && player.getZone() == Zone::InsideHouse && player.getInterior() && player.getInterior()->interior_id == InteriorId::PlayerHouse)
        house = find(*player.getInterior());
    if (!house)
        house = playerId >= 1 && playerId <= 4 ? find({InteriorId::PlayerHouse, static_cast<uint8_t>(playerId - 1)}) : nullptr;
    if (!house || house->beds.empty()) {
        const auto fallback = std::find_if(rooms.begin(), rooms.end(), [](const auto& room) { return room.location.interior_id == InteriorId::PlayerHouse && !room.beds.empty(); });
        if (fallback == rooms.end())
            return std::nullopt;
        house = &*fallback;
    }
    const auto walking = walkingMap(*house);
    for (const auto& bed: house->beds) {
        const Position foot{bed.getX(), bed.getY() + house->terrain.getCellSize()};
        const auto canRecover = [&](const Position& pos) {
            return canWalk(house->location, player.getCollisionBounds(pos)) && walking.hasBasePath(house->entry, pos) && canApproachBed(*house, pos, bed);
        };
        if (canRecover(foot)) {
            return WorldLocation{Zone::InsideHouse, 0, house->location, foot};
        }
        for (const auto& pos: walking.walkableBasePositions()) {
            if (canRecover(pos)) {
                return WorldLocation{Zone::InsideHouse, 0, house->location, pos};
            }
        }
    }
    return std::nullopt;
}

InteriorData InteriorManager::dataFor(const InteriorLocation& location) const {
    InteriorData result{location.interior_id, location.house_index, {}, {}};
    const auto found = storage.find({location.interior_id, location.house_index});
    if (found != storage.end()) {
        result.furniture = found->second.furniture();
        result.loot = found->second.getGroundItems().getPublicData();
    }
    return result;
}

void InteriorManager::completeMap(ConstMapData& map) const {
    for (const auto& room: rooms) {
        BackgroundData background = room.background;
        for (const auto& bed: room.beds) {
            background.furniture.push_back({bed, FurnitureId::Bed, Direction::None});
            background.furniture.push_back({bed, FurnitureId::BedSheet, Direction::None});
        }
        switch (room.location.interior_id) {
            case InteriorId::PlayerHouse:
                if (map.farm_houses.size() <= room.location.house_index) {
                    map.farm_houses.resize(room.location.house_index + 1);
                }
                map.farm_houses[room.location.house_index] = std::move(background);
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

namespace {
std::optional<std::pair<InteriorId, uint8_t>> houseKey(const PlayerSnapshot& player) {
    if (player.getZone() != Zone::InsideHouse || !player.getInterior() || player.getInterior()->interior_id != InteriorId::PlayerHouse)
        return std::nullopt;
    return std::pair{player.getInterior()->interior_id, player.getInterior()->house_index};
}
}  // namespace

bool InteriorManager::interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    const auto key = houseKey(players.snapshot(playerId));
    if (!key)
        return false;
    const auto found = storage.find(*key);
    if (found == storage.end())
        return false;
    return found->second.interact(players, playerId, direction, ids, now).has_value();
}

std::optional<OpenChestData> InteriorManager::openChestFor(const PlayerSnapshot& player) const {
    const auto key = houseKey(player);
    if (!key)
        return std::nullopt;
    const auto found = storage.find(*key);
    return found == storage.end() ? std::nullopt : found->second.contentsFor(player);
}

void InteriorManager::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    const auto key = houseKey(players.snapshot(playerId));
    if (key && storage.contains(*key))
        storage.at(*key).deposit(players, playerId, command);
}

void InteriorManager::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    const auto key = houseKey(players.snapshot(playerId));
    if (key && storage.contains(*key))
        storage.at(*key).withdraw(players, playerId, command);
}

void InteriorManager::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    const auto key = houseKey(players.snapshot(playerId));
    if (key && storage.contains(*key))
        storage.at(*key).collectNearbyItems(players, playerId);
}

std::vector<InteriorData> InteriorManager::storageData() const {
    std::vector<InteriorData> result;
    for (const auto& [key, chests]: storage) result.push_back(dataFor({key.first, key.second}));
    return result;
}

void InteriorManager::saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const {
    for (const auto& [key, area]: storage) {
        for (const auto& [id, chest]: area.getChests()) inventories.emplace(id, chest.getInventory().getItems());
    }
}

void InteriorManager::restoreStorage(const std::vector<InteriorData>& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) {
    auto prepared = storage;
    for (const auto& room: saved) {
        const auto found = prepared.find({room.interior_id, room.house_index});
        if (room.interior_id != InteriorId::PlayerHouse || found == prepared.end())
            throw std::runtime_error("Invalid saved house storage");
        found->second.restore(room.furniture, inventories, room.loot);
    }
    storage = std::move(prepared);
}

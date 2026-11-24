#include "interior.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include "commerce/shop_products.h"
#include "entities/entity_id_generator.h"
#include "player/player_manager.h"

namespace {
const CollisionBox BED_BOX{22, 32};
constexpr int OFFSET_Y  = -22;

bool validPosition(const Position& position) {
    return position.getX() >= 0 && position.getY() >= 0 && position.getX() <= std::numeric_limits<uint16_t>::max() &&
        position.getY() <= std::numeric_limits<uint16_t>::max();
}
}  // namespace

Interior::Interior(const InteriorLayout& layout):
    location(layout.location), outside(layout.outside), extraOutside(layout.extraOutside), returnPosition(layout.returnPosition),
    entry(layout.entry), exit(layout.exit), beds(layout.beds), background(layout.background) {
    space.setMap(layout.terrain);
}

bool Interior::validLayout(const InteriorLayout& room) {
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    if (room.location.interior_id > InteriorId::ComunityCenter || (room.location.interior_id == InteriorId::PlayerHouse && room.location.house_index >= 4) ||
        (room.location.interior_id != InteriorId::PlayerHouse && room.location.house_index != 0) ||
        (room.outside.zone != Zone::Farm && room.outside.zone != Zone::Town) || room.outside.level != 0 || room.outside.interior ||
        !validPosition(room.outside.position) || !validPosition(room.returnPosition) || !validPosition(room.entry) || !validPosition(room.exit) ||
        !room.terrain.isWalkable(playerBox.at(room.entry)) || !room.terrain.isWalkable(playerBox.at(room.exit)) || !room.terrain.hasBasePath(room.entry, room.exit))
        return false;
    if (room.background.interior && room.background.interior->npc) {
        const auto& npc = *room.background.interior->npc;
        const auto expected = shop_products::merchantForInterior(room.location.interior_id);
        const auto bounds = playerBox.at(npc.pos);
        if (!expected || *expected != npc.id || !validPosition(npc.pos) || !room.terrain.isWalkable(bounds) || bounds.intersects(playerBox.at(room.entry)) ||
            bounds.intersects(playerBox.at(room.exit)))
            return false;
    }
    std::vector<CollisionBounds> occupiedBeds;
    for (const auto& bed: room.beds) {
        const auto bounds = BED_BOX.at(bed);
        if (!validPosition(bed) || !room.terrain.isWalkable(bounds) ||
            (room.background.interior && room.background.interior->npc && bounds.intersects(playerBox.at(room.background.interior->npc->pos))) ||
            bounds.intersects(playerBox.at(room.entry)) || bounds.intersects(playerBox.at(room.exit)) ||
            std::any_of(occupiedBeds.begin(), occupiedBeds.end(), [&](const auto& other) { return bounds.intersects(other); }))
            return false;
        occupiedBeds.push_back(bounds);
    }
    return true;
}

bool Interior::canApproachBed(const Position& position, const Position& bed) const {
    const int dx = bed.getX() - position.getX();
    const int dy = bed.getY() - position.getY();
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps > 22)
        return false;
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    for (int step = 0; step <= steps; ++step) {
        const Position point{position.getX() + dx * step / std::max(1, steps), position.getY() + dy * step / std::max(1, steps)};
        if (!space.getMap().isWalkable(playerBox.at(point)))
            return false;
    }
    return true;
}

ZoneMap Interior::walkingMap() const {
    const auto& terrain = space.getMap();
    const CollisionBox playerBox{entity_dimensions::PLAYER};
    const auto positions = terrain.walkableBasePositions();
    int rows = 0;
    int columns = 0;
    for (const auto& position: positions) {
        const auto cell = terrain.baseCellAt(position).value();
        rows = std::max(rows, cell.first + 1);
        columns = std::max(columns, cell.second + 1);
    }
    std::vector<std::vector<bool>> floor(rows, std::vector<bool>(columns, false));
    for (const auto& position: positions) {
        const auto cell = terrain.baseCellAt(position).value();
        floor[cell.first][cell.second] = terrain.isWalkable(playerBox.at(position)) &&
            (!background.interior || !background.interior->npc || !playerBox.at(background.interior->npc->pos).intersects(playerBox.at(position))) &&
            std::none_of(beds.begin(), beds.end(), [&](const auto& bed) { return BED_BOX.at(bed).intersects(playerBox.at(position)); });
    }
    return ZoneMap{floor, terrain.getCellSize()};
}

bool Interior::hasAccessibleBeds() const {
    const auto walking = walkingMap();
    if (!walking.hasBasePath(entry, exit))
        return false;
    const auto positions = walking.walkableBasePositions();
    return std::all_of(beds.begin(), beds.end(), [&](const auto& bed) {
        return std::any_of(positions.begin(), positions.end(), [&](const auto& position) {
            return walking.hasBasePath(entry, position) && canApproachBed(position, bed);
        });
    });
}

std::vector<CollisionBounds> Interior::bedObstacles() const {
    std::vector<CollisionBounds> result;
    for (const auto& bed: beds)
        result.push_back(BED_BOX.at(bed));
    return result;
}

bool Interior::initializeStorage(const std::vector<ChestSpawn>& initial, EntityIdGenerator& ids) {
    if (location.interior_id != InteriorId::PlayerHouse)
        return initial.empty();
    if (!space.setReservedPositions({entry, exit}))
        return false;
    space.setObstacles(bedObstacles());
    for (const auto& spawn: initial) {
        try {
            if (ids.getNextId() == 0 || !chests.place(ids.getNextId(), spawn.base, space, spawn.items))
                return false;
        } catch (const std::invalid_argument&) {
            return false;
        }
        ids.generate();
    }
    return true;
}

const InteriorLocation& Interior::getLocation() const { return location; }
const WorldLocation& Interior::getOutside() const { return outside; }
const std::vector<Position>& Interior::getExtraOutside() const { return extraOutside; }
const Position& Interior::getReturnPosition() const { return returnPosition; }
const Position& Interior::getEntry() const { return entry; }
bool Interior::hasBeds() const { return !beds.empty(); }
bool Interior::sameBaseCell(const Position& first, const Position& second) const { return space.getMap().sameBaseCell(first, second); }

bool Interior::canWalk(const CollisionBounds& bounds) const {
    return space.getMap().isWalkable(bounds) && space.canWalk(bounds) &&
        (!background.interior || !background.interior->npc || !CollisionBox{entity_dimensions::PLAYER}.at(background.interior->npc->pos).intersects(bounds)) &&
        std::none_of(beds.begin(), beds.end(), [&](const auto& bed) { return BED_BOX.at(bed).intersects(bounds); });
}

bool Interior::canMove(const PlayerSnapshot& player, const Position& next) const {
    if (player.isSleeping() && next.getX() == player.getPosition().getX() && next.getY() == player.getPosition().getY() &&
        std::any_of(beds.begin(), beds.end(), [&](const auto& bed) { return bed.getX() == next.getX() && bed.getY() == next.getY(); }))
        return true;
    return canWalk(player.getCollisionBounds(next));
}

bool Interior::useBed(PlayerManager& players, uint16_t playerId) const {
    const auto player = players.snapshot(playerId);
    if (!player.canAct())
        return false;
    for (const auto& bed: beds) {
        if (canApproachBed(player.getPosition(), bed))
            return players.apply(playerId, player_action::Sleep{Position{bed.getX(), bed.getY() + OFFSET_Y}}).applied;
    }
    return false;
}

std::optional<WorldLocation> Interior::recoveryFor(const PlayerSnapshot& player) const {
    const auto walking = walkingMap();
    for (const auto& bed: beds) {
        const Position foot{bed.getX(), bed.getY() + space.getMap().getCellSize()};
        const auto canRecover = [&](const Position& position) {
            return canWalk(player.getCollisionBounds(position)) && walking.hasBasePath(entry, position) && canApproachBed(position, bed);
        };
        if (canRecover(foot))
            return WorldLocation{Zone::InsideHouse, 0, location, foot};
        for (const auto& position: walking.walkableBasePositions()) {
            if (canRecover(position))
                return WorldLocation{Zone::InsideHouse, 0, location, position};
        }
    }
    return std::nullopt;
}

std::optional<NpcData> Interior::nearbyMerchant(const PlayerSnapshot& player, Direction direction) const {
    if (!player.canAct() || direction > Direction::Right || !background.interior || !background.interior->npc)
        return std::nullopt;
    const auto& npc = *background.interior->npc;
    if (!player.canReach(npc.pos, 32))
        return std::nullopt;
    const auto& origin = player.getPosition();
    const int dx = npc.pos.getX() - origin.getX();
    const int dy = npc.pos.getY() - origin.getY();
    const bool facing = direction == Direction::None || (direction == Direction::Up && dy < 0 && std::abs(dx) <= -dy) ||
        (direction == Direction::Down && dy > 0 && std::abs(dx) <= dy) || (direction == Direction::Left && dx < 0 && std::abs(dy) <= -dx) ||
        (direction == Direction::Right && dx > 0 && std::abs(dy) <= dx);
    if (!facing)
        return std::nullopt;
    // Merchant sampling uses integer truncation; preserve it independently of tool targeting.
    const int steps = std::max(std::abs(dx), std::abs(dy));
    Position previous{origin.getX(), origin.getY() - 1};
    const auto& terrain = space.getMap();
    for (int step = 0; step <= steps; ++step) {
        const Position point{origin.getX() + dx * step / std::max(1, steps), origin.getY() - 1 + dy * step / std::max(1, steps)};
        if (!terrain.isWalkable(point) || !terrain.isWalkable(Position{point.getX(), previous.getY()}) || !terrain.isWalkable(Position{previous.getX(), point.getY()}))
            return std::nullopt;
        previous = point;
    }
    return npc;
}

BackgroundData Interior::backgroundData() const {
    auto result = background;
    for (const auto& bed: beds) {
        result.furniture.push_back({bed, FurnitureId::Bed, Direction::None});
        result.furniture.push_back({bed, FurnitureId::BedSheet, Direction::None});
    }
    return result;
}

InteriorData Interior::data() const {
    return InteriorData{location.interior_id, location.house_index, chests.furnitureData(), groundItems.getPublicData()};
}

bool Interior::interactWithChest(PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    if (location.interior_id != InteriorId::PlayerHouse)
        return false;
    space.setObstacles(bedObstacles());
    // A recognized but rejected chest action still takes priority over sleeping.
    return chests.interact(players, playerId, space, space.getMap(), groundItems, direction, ids, now).has_value();
}

std::optional<OpenChestData> Interior::openChestFor(const PlayerSnapshot& player) const {
    return location.interior_id == InteriorId::PlayerHouse ? chests.contentsFor(player, space.getMap()) : std::nullopt;
}

void Interior::depositInChest(PlayerManager& players, uint16_t playerId, const ItemTransferDTO& command) {
    if (location.interior_id == InteriorId::PlayerHouse)
        chests.deposit(players, playerId, space.getMap(), command);
}

void Interior::withdrawFromChest(PlayerManager& players, uint16_t playerId, const SelectItemDTO& command) {
    if (location.interior_id == InteriorId::PlayerHouse)
        chests.withdraw(players, playerId, space.getMap(), command);
}

void Interior::collectNearbyItems(PlayerManager& players, uint16_t playerId) {
    if (location.interior_id != InteriorId::PlayerHouse)
        return;
    const auto player = players.snapshot(playerId);
    groundItems.collect(players, playerId, [&](const auto& position) { return space.getMap().hasClearPath(player.getPosition(), position); });
}

void Interior::saveInventories(std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) const {
    inventories.merge(chests.inventories());
}

void Interior::restoreStorage(const InteriorData& saved, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories) {
    chests = ChestManager{};
    space.clearOccupants();
    space.setObstacles(bedObstacles());
    for (const auto& item: saved.furniture) {
        if (item.type != FurnitureId::Chest)
            continue;
        const auto contents = inventories.find(item.id);
        if (contents == inventories.end() || !chests.place(item.id, item.pos, space, contents->second))
            throw std::runtime_error("Invalid saved chest furniture or contents");
    }
    groundItems.restoreItems(saved.loot);
}

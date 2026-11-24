#include "dungeon_room.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "dungeon_config.h"

DungeonRoom::DungeonRoom(uint8_t number): number(number) {
    if (number < 1 || number > DUNGEON_ROOM_COUNT)
        throw std::invalid_argument("Room number must be between 1 and 10");
}

bool DungeonRoom::initialize(
    const ZoneMap& map, const DungeonRoomPassages& data, const std::vector<Position>& reserved, const std::vector<ChestSpawn>& initial, EntityIdGenerator& ids) {
    DungeonRoom prepared(number);
    auto preparedIds = ids;
    if (map.isEmpty() || !geometry.arePassagesValid(number, map, data) || !prepared.space.setMap(map))
        return false;
    prepared.passages = data;
    auto positions = reserved;
    positions.push_back(data.entry);
    positions.push_back(data.exit);
    if (data.nextRoom)
        positions.push_back(*data.nextRoom);
    if (!prepared.space.setReservedPositions(positions))
        return false;
    for (const auto& spawn: initial) {
        try {
            if (preparedIds.getNextId() == 0 || !prepared.chests.place(preparedIds.getNextId(), spawn.base, prepared.space, spawn.items))
                return false;
        } catch (const std::invalid_argument&) {
            return false;
        }
        preparedIds.generate();
    }
    *this = std::move(prepared);
    ids = preparedIds;
    return true;
}

uint8_t DungeonRoom::getNumber() const { return number; }
const ZoneMap& DungeonRoom::getMap() const { return space.getMap(); }
const std::optional<DungeonRoomPassages>& DungeonRoom::getPassages() const { return passages; }

bool DungeonRoom::addMineral(Mineral mineral) {
    if (!space.canWalk(mineral.getCollisionBounds()) || (passages && geometry.isPassageCell(getMap(), *passages, mineral.getPosition())))
        return false;
    return minerals.add(std::move(mineral), getMap());
}

bool DungeonRoom::removeMineral(EntityId id) { return minerals.remove(id, getMap()); }
Mineral* DungeonRoom::findMineral(EntityId id) { return minerals.find(id); }
const Mineral* DungeonRoom::findMineral(EntityId id) const { return minerals.find(id); }
const std::map<EntityId, Mineral>& DungeonRoom::getMinerals() const { return minerals.all(); }
bool DungeonRoom::canWalk(const CollisionBounds& bounds) const { return geometry.canWalk(getMap(), minerals, bounds) && space.canWalk(bounds); }
void DungeonRoom::removeBrokenMinerals() { minerals.removeBroken(getMap()); }

bool DungeonRoom::hasClearLine(const Position& from, const Position& target) const { return geometry.hasClearLine(getMap(), minerals, from, target); }

std::optional<MineData> DungeonRoom::getMineData(bool includeBroken) const {
    if (!passages)
        return std::nullopt;
    MineData mine;
    mine.current_level = number;
    auto& level = mine.current_level_data;
    level.entry = passages->entry;
    level.exit = passages->exit;
    level.minerals = minerals.getPublicData(includeBroken);
    level.combat = getCombatData();
    level.furniture = chests.furnitureData();
    const auto dropped = storageItems.getPublicData();
    level.combat.loot.insert(level.combat.loot.end(), dropped.begin(), dropped.end());
    return mine;
}

DungeonCombatData DungeonRoom::getCombatData() const {
    DungeonCombatData data;
    data.room = number;
    for (const auto& [id, enemy]: combat.enemies) data.enemies.push_back(enemy.getPublicData());
    for (const auto& [id, projectile]: combat.projectiles) data.projectiles.push_back(projectile.data);
    for (const auto& [id, trap]: combat.traps) data.traps.push_back(trap.data);
    data.loot = groundItems.getPublicData();
    return data;
}

RoomCombat& DungeonRoom::getCombat() { return combat; }
const RoomCombat& DungeonRoom::getCombat() const { return combat; }
GroundItems& DungeonRoom::getGroundItems() { return groundItems; }
const GroundItems& DungeonRoom::getGroundItems() const { return groundItems; }
ChestManager& DungeonRoom::getChests() { return chests; }
const ChestManager& DungeonRoom::getChests() const { return chests; }
GroundItems& DungeonRoom::getStorageItems() { return storageItems; }
const GroundItems& DungeonRoom::getStorageItems() const { return storageItems; }

std::optional<bool> DungeonRoom::interactWithChest(
    PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    std::vector<CollisionBounds> obstacles;
    for (const auto& [id, mineral]: minerals.all()) {
        if (!mineral.isDepleted())
            obstacles.push_back(mineral.getCollisionBounds());
    }
    for (const auto& [id, enemy]: combat.enemies) {
        if (enemy.alive())
            obstacles.push_back(enemy.bounds(enemy.getPosition()));
    }
    for (const auto& [id, trap]: combat.traps) obstacles.push_back(CollisionBox{16, trap.data.type == TrapType::Spikes ? 16 : 8}.at(trap.data.pos));
    space.setObstacles(obstacles);
    return chests.interact(players, playerId, space, getMap(), storageItems, direction, ids, now);
}

void DungeonRoom::clearChestOverlaps(const std::vector<FurnitureData>& furniture) {
    const auto overlapsChest = [&](const CollisionBounds& bounds) {
        return std::any_of(furniture.begin(), furniture.end(),
            [&](const auto& item) { return item.type == FurnitureId::Chest && bounds.intersects(CollisionBox{entity_dimensions::DECORATION}.at(item.pos)); });
    };
    std::vector<EntityId> displacedMinerals;
    for (const auto& [id, mineral]: minerals.all()) {
        if (overlapsChest(mineral.getCollisionBounds()))
            displacedMinerals.push_back(id);
    }
    for (const auto id: displacedMinerals) removeMineral(id);
    std::erase_if(combat.enemies, [&](const auto& entry) { return overlapsChest(entry.second.bounds(entry.second.getPosition())); });
    std::erase_if(combat.traps, [&](const auto& entry) { return overlapsChest(CollisionBox{16, entry.second.data.type == TrapType::Spikes ? 16 : 8}.at(entry.second.data.pos)); });
}

void DungeonRoom::restoreChests(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot) {
    auto preparedSpace = space;
    preparedSpace.clearOccupants();
    preparedSpace.setObstacles({});
    ChestManager preparedChests;
    for (const auto& item: furniture) {
        if (item.type != FurnitureId::Chest)
            continue;
        const auto contents = inventories.find(item.id);
        if (contents == inventories.end() || !preparedChests.place(item.id, item.pos, preparedSpace, contents->second))
            throw std::runtime_error("Invalid saved chest furniture or contents");
    }
    GroundItems preparedItems;
    preparedItems.restoreItems(loot);
    clearChestOverlaps(furniture);
    space = std::move(preparedSpace);
    chests = std::move(preparedChests);
    storageItems = std::move(preparedItems);
}

#include "dungeon_room.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "dungeon/dungeon_config.h"

DungeonRoom::DungeonRoom(uint8_t number) : number(number) {
    if (number < 1 || number > DUNGEON_ROOM_COUNT) {
        throw std::invalid_argument("Room number must be between 1 and 10");
    }
}

uint8_t DungeonRoom::getNumber() const { 
    return number; 
}

bool DungeonRoom::addMineral(Mineral mineral) {
    if (!storage.canWalk(CollisionBox{entity_dimensions::MINERAL}.at(mineral.getPosition())))
        return false;
    if (passages && geometry.isPassageCell(terrain, *passages, mineral.getPosition())) {
        return false;
    }
    return minerals.add(std::move(mineral), terrain);
}

bool DungeonRoom::removeMineral(EntityId id) {
     return minerals.remove(id, terrain); 
    }

Mineral* DungeonRoom::findMineral(EntityId id) { 
    return minerals.find(id); 
}

const Mineral* DungeonRoom::findMineral(EntityId id) const { 
    return minerals.find(id); 
}

const std::map<EntityId, Mineral>& DungeonRoom::getMinerals() const { 
    return minerals.all(); 
}

const ZoneMap& DungeonRoom::getMap() const { 
    return terrain; 
}

bool DungeonRoom::canWalk(const CollisionBounds& bounds) const { return geometry.canWalk(terrain, minerals, bounds) && storage.canWalk(bounds); }

void DungeonRoom::removeBrokenMinerals() { 
    minerals.removeBroken(terrain); 
}

bool DungeonRoom::setPassages(const DungeonRoomPassages& data) {
    if (!geometry.arePassagesValid(number, terrain, data)) {
        return false;
    }

    for (const auto& [id, mineral] : minerals.all()) {
        if (geometry.isPassageCell(terrain, data, mineral.getPosition())) {
            return false;
        }
    }
    passages = data;
    return true;
}

const std::optional<DungeonRoomPassages>& DungeonRoom::getPassages() const { 
    return passages; 
}

bool DungeonRoom::hasClearLine(const Position& from, const Position& target) const {
    return geometry.hasClearLine(terrain, minerals, from, target);
}

std::optional<MineData> DungeonRoom::getMineData(bool includeBroken) const {
    if (!passages) {
        return std::nullopt;
    }
    MineData mine;
    mine.current_level = number;
    auto& level = mine.current_level_data;
    level.entry = passages->entry;
    level.exit = passages->exit;
    level.minerals = minerals.getPublicData(includeBroken);
    level.combat = getCombatData();
    level.furniture = storage.furniture();
    const auto dropped = storage.getGroundItems().getPublicData();
    level.combat.loot.insert(level.combat.loot.end(), dropped.begin(), dropped.end());
    return mine;
}

DungeonCombatData DungeonRoom::getCombatData() const {
    DungeonCombatData data;
    data.room = number;
    for (const auto& [id, enemy]: combat.enemies) {
        data.enemies.push_back(enemy.data);
    }
    for (const auto& [id, projectile]: combat.projectiles) {
        data.projectiles.push_back(projectile.data);
    }
    for (const auto& [id, trap]: combat.traps) {
        data.traps.push_back(trap.data);
    }
    data.loot = groundItems.getPublicData();
    return data;
}

RoomCombat& DungeonRoom::getCombat() { return combat; }
const RoomCombat& DungeonRoom::getCombat() const { return combat; }

GroundItems& DungeonRoom::getGroundItems() { return groundItems; }
const GroundItems& DungeonRoom::getGroundItems() const { return groundItems; }

bool DungeonRoom::initializeChests(const std::vector<ChestSpawn>& initial, const std::vector<Position>& reserved, EntityIdGenerator& ids) {
    if (!passages)
        return false;
    auto positions = reserved;
    positions.push_back(passages->entry);
    positions.push_back(passages->exit);
    if (passages->nextRoom)
        positions.push_back(*passages->nextRoom);
    return storage.initialize(terrain, positions, {}, initial, ids);
}

std::optional<bool> DungeonRoom::interactWithChest(
    PlayerManager& players, uint16_t playerId, Direction direction, EntityIdGenerator& ids, std::chrono::steady_clock::time_point now) {
    std::vector<CollisionBounds> obstacles;
    for (const auto& [id, mineral]: minerals.all()) {
        if (!mineral.isDepleted())
            obstacles.push_back(mineral.getCollisionBounds());
    }
    for (const auto& [id, enemy]: combat.enemies) {
        if (enemy.alive())
            obstacles.push_back(enemy.bounds(enemy.data.pos));
    }
    for (const auto& [id, trap]: combat.traps) obstacles.push_back(CollisionBox{16, trap.data.type == TrapType::Spikes ? 16 : 8}.at(trap.data.pos));
    return storage.interact(players, playerId, direction, ids, now, obstacles);
}

void DungeonRoom::restoreChests(const std::vector<FurnitureData>& furniture, const std::map<uint32_t, std::map<uint8_t, Stack>>& inventories, const std::vector<LootData>& loot) {
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
    std::erase_if(combat.enemies, [&](const auto& entry) { return overlapsChest(entry.second.bounds(entry.second.data.pos)); });
    std::erase_if(combat.traps, [&](const auto& entry) { return overlapsChest(CollisionBox{16, entry.second.data.type == TrapType::Spikes ? 16 : 8}.at(entry.second.data.pos)); });
    storage.restore(furniture, inventories, loot);
}

ChestRoom& DungeonRoom::getStorage() { return storage; }
const ChestRoom& DungeonRoom::getStorage() const { return storage; }

bool DungeonRoom::setMap(const ZoneMap& map) {
    if (map.isEmpty() || (passages && !geometry.arePassagesValid(number, map, *passages))) {
        return false;
    }
    if (passages) {
        for (const auto& [id, mineral] : minerals.all()) {
            if (geometry.isPassageCell(map, *passages, mineral.getPosition())) {
                return false;
            }
        }
    }
    ZoneMap prepared = map;
    if (!minerals.reindex(prepared)) {
        return false;
    }
    terrain = std::move(prepared);
    return true;
}

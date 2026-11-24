#include "dungeon_loader.h"

#include <algorithm>
#include <utility>

#include "dungeon/combat/dungeon_combat.h"
#include "dungeon/mining/mineral_generator.h"
#include "dungeon/rooms/dungeon_rooms.h"
#include "entities/entity_id_generator.h"

namespace {
std::map<uint8_t, Stack> chestLoot(std::mt19937& random) {
    const ItemId valuables[]{ItemId::Lapis, ItemId::Silver, ItemId::Gold};
    return {
        {0, {ItemId::Coal, std::uniform_int_distribution<uint16_t>{2, 5}(random), Quality::None}},
        {1, {valuables[std::uniform_int_distribution<int>{0, 2}(random)], std::uniform_int_distribution<uint16_t>{1, 3}(random), Quality::None}}
    };
}
}  // namespace

DungeonLoader::DungeonLoader(const DungeonConfig& config, uint32_t seed):
    densityPercent(config.mineralDensityPercent), densitiesByRoom(config.mineralDensityByRoom), random(seed) {}

bool DungeonLoader::initialize(
    const std::map<uint8_t, DungeonRoomLayout>& layout, DungeonRooms& rooms, MineralGenerator& generator, DungeonCombat& combat, EntityIdGenerator& ids) {
    DungeonRooms preparedRooms;
    auto preparedGenerator = generator;
    auto preparedCombat = combat;
    auto preparedIds = ids;
    auto preparedRandom = random;
    auto preparedLayouts = layout;
    for (auto& [number, roomLayout]: preparedLayouts) {
        roomLayout.reservedPositions.insert(roomLayout.reservedPositions.end(), roomLayout.passages.extraExits.begin(), roomLayout.passages.extraExits.end());
        for (const auto& trap: roomLayout.traps) roomLayout.reservedPositions.push_back(trap.pos);
        if (number == DUNGEON_ROOM_COUNT && !roomLayout.passages.nextRoom) {
            auto candidates = roomLayout.terrain.walkableBasePositions();
            std::reverse(candidates.begin(), candidates.end());
            for (const auto& position: candidates) {
                const auto& map = roomLayout.terrain;
                const auto& passages = roomLayout.passages;
                const auto bounds = CollisionBox{entity_dimensions::DECORATION}.at(position);
                if (!map.isWalkable(bounds) || map.sameBaseCell(position, passages.entry) || map.sameBaseCell(position, passages.exit) ||
                    !map.hasBasePath(passages.entry, position) ||
                    std::any_of(roomLayout.reservedPositions.begin(), roomLayout.reservedPositions.end(), [&](const auto& reserved) { return map.sameBaseCell(position, reserved); }) ||
                    std::any_of(roomLayout.chests.begin(), roomLayout.chests.end(), [&](const auto& chest) { return bounds.intersects(CollisionBox{entity_dimensions::DECORATION}.at(chest.base)); }))
                    continue;
                roomLayout.passages.nextRoom = position;
                break;
            }
        }
    }
    if (!load(preparedLayouts, preparedRooms, preparedGenerator, preparedIds, preparedRandom) || !preparedCombat.initialize(preparedRooms, preparedLayouts, preparedIds))
        return false;
    
    rooms = std::move(preparedRooms);
    generator = std::move(preparedGenerator);
    combat = std::move(preparedCombat);
    ids = preparedIds;
    random = preparedRandom;
    return true;
}

bool DungeonLoader::load(
    const std::map<uint8_t, DungeonRoomLayout>& layout, DungeonRooms& rooms, MineralGenerator& generator, EntityIdGenerator& ids, std::mt19937& placementRandom) const {
    if (layout.size() != DUNGEON_ROOM_COUNT || densityPercent > 100)
        return false;
    for (const auto& [number, density]: densitiesByRoom) {
        if (number < 1 || number > DUNGEON_ROOM_COUNT || density > 100)
            return false;
    }
    for (auto& [number, room]: rooms) {
        const auto it = layout.find(number);
        if (it == layout.end())
            return false;
        const auto& data = it->second;
        auto initialChests = data.chests;
        if (!initialChests.empty() && std::none_of(initialChests.begin(), initialChests.end(), [](const auto& chest) { return !chest.items.empty(); }))
            initialChests.front().items = chestLoot(placementRandom);
        if (!room.initialize(data.terrain, data.passages, data.reservedPositions, initialChests, ids))
            return false;
        if (data.chests.empty()) {
            const auto candidates = placement.choose(room, 100, data.reservedPositions, placementRandom);
            if (!candidates)
                return false;
            const auto loot = chestLoot(placementRandom);
            bool placed = false;
            for (const auto& position: *candidates) {
                if (!data.terrain.hasBasePath(data.passages.entry, position))
                    continue;
                if (room.initialize(data.terrain, data.passages, data.reservedPositions, {{position, loot}}, ids)) {
                    placed = true;
                    break;
                }
            }
            if (!placed)
                return false;
        }
    }
    for (auto& [number, room]: rooms) {
        const auto override = densitiesByRoom.find(number);
        const auto density = override == densitiesByRoom.end() ? densityPercent : override->second;
        const auto positions = placement.choose(room, density, layout.at(number).reservedPositions, placementRandom);
        if (!positions)
            return false;
        for (const auto& position: *positions) {
            if (ids.getNextId() == 0)
                return false;
            auto mineral = generator.create(ids.generate(), number, position);
            if (!mineral || !room.addMineral(std::move(*mineral)))
                return false;
        }
    }
    return true;
}

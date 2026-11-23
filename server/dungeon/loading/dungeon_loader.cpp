#include "dungeon_loader.h"

#include <utility>

#include "dungeon/combat/dungeon_combat.h"
#include "dungeon/mining/mineral_generator.h"
#include "dungeon/rooms/dungeon_rooms.h"
#include "entities/entity_id_generator.h"

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
        for (const auto& trap: roomLayout.traps) roomLayout.reservedPositions.push_back(trap.pos);
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
        if (!room.initialize(data.terrain, data.passages, data.reservedPositions, data.chests, ids))
            return false;
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

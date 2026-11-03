#include "dungeon_loader.h"

#include <utility>
#include "dungeon/mining/mineral_generator.h"
#include "entities/entity_id_generator.h"

DungeonLoader::DungeonLoader(const DungeonConfig& config): densityPercent(config.mineralDensityPercent),densitiesByRoom(config.mineralDensityByRoom) {}

bool DungeonLoader::load(const std::map<uint8_t, DungeonRoomLayout>& layout,std::map<uint8_t, DungeonRoom>& rooms,MineralGenerator& generator,EntityIdGenerator& entityIds,std::mt19937& random) const {
    if (layout.size() != DUNGEON_ROOM_COUNT || densityPercent > 100) {
        return false;
    }

    for (const auto& [number, density] : densitiesByRoom) {
        if (number < 1 || number > DUNGEON_ROOM_COUNT || density > 100) {
            return false;
        }
    }

    std::map<uint8_t, DungeonRoom> preparedRooms;

    for (uint8_t number = 1; number <= DUNGEON_ROOM_COUNT; ++number) {
        const auto it = layout.find(number);
        if (it == layout.end()) {
            return false;
        }

        auto& room = preparedRooms.try_emplace(number, number).first->second;
        if (!room.setMap(it->second.terrain) ||!room.setPassages(it->second.passages)) {
            return false;
        }
    }

    auto preparedGenerator = generator;
    auto preparedIds = entityIds;
    auto preparedRandom = random;

    for (auto& [number, room] : preparedRooms) {
        const auto override = densitiesByRoom.find(number);
        const auto density = override == densitiesByRoom.end()? densityPercent: override->second;
        const auto positions = placement.choose(room, density, layout.at(number).reservedPositions,preparedRandom);

        if (!positions) {
            return false;
        }

        for (const auto& position : *positions) {
            if (preparedIds.getNextId() == 0) {
                return false;
            }

            auto mineral = preparedGenerator.create(preparedIds.generate(), number, position);

            if (!mineral || !room.addMineral(std::move(*mineral))) {
                return false;
            }
        }
    }

    rooms = std::move(preparedRooms);
    generator = std::move(preparedGenerator);
    entityIds = preparedIds;
    random = preparedRandom;
    return true;
}
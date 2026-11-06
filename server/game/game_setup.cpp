#include "game_setup.h"

#include <utility>
#include "entities/entity_dimensions.h"

GameSetup makeDefaultGameSetup(uint8_t worldId) {
    GameSetup setup;
    setup.worldId = worldId;
    const ZoneMap terrain(std::vector<std::vector<bool>>(24, std::vector<bool>(32, true)), entity_dimensions::TILE_SIZE);
    for (uint8_t level = 1; level <= DUNGEON_ROOM_COUNT; ++level) {
        DungeonRoomLayout room;
        room.terrain = terrain;
        room.passages.entry = terrain.baseAt({3, 3}).value();
        room.passages.exit = terrain.baseAt({3, 1}).value();
        if (level < DUNGEON_ROOM_COUNT) {
            room.passages.nextRoom = terrain.baseAt({21, 29}).value();
        }
        setup.layout.emplace(level, std::move(room));
    }
    setup.entrance.terrain = terrain;
    setup.entrance.position = terrain.baseAt({3, 3}).value();
    setup.entrance.returnPosition = terrain.baseAt({4, 3}).value();
    setup.farm.terrain = terrain;
    setup.farm.spawn = setup.entrance.returnPosition;
    setup.farm.reservedPositions = {setup.farm.spawn, setup.entrance.position};
    return setup;
}

#include "game_setup.h"

#include <utility>

GameSetup makeDefaultGameSetup(uint8_t worldId) {
    GameSetup setup;
    setup.worldId = worldId;
    const ZoneMap terrain(std::vector<std::vector<bool>>(24, std::vector<bool>(32, true)), 16);
    for (uint8_t level = 1; level <= DUNGEON_ROOM_COUNT; ++level) {
        DungeonRoomLayout room;
        room.terrain = terrain;
        room.passages.entry = Position(48, 48);
        room.passages.exit = Position(16, 48);
        if (level < DUNGEON_ROOM_COUNT) {
            room.passages.nextRoom = Position(464, 336);
        }
        setup.layout.emplace(level, std::move(room));
    }
    setup.entrance.terrain = terrain;
    setup.entrance.position = Position(48, 48);
    setup.entrance.returnPosition = Position(48, 64);
    return setup;
}

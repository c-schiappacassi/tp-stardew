target_sources(taller_server
    PRIVATE
    # .cpp files
        main.cpp
        network/acceptor.cpp
        network/client_handler.cpp
        network/client_sender.cpp
        network/server_protocol.cpp
        game/game.cpp
        game/game_manager.cpp
        inventory/inventory.cpp
        player/player.cpp
        dungeon/mineral.cpp
        dungeon/dungeon_room.cpp
        dungeon/dungeon.cpp
        entities/entity_id_generator.cpp
        world/zone_map.cpp
    PUBLIC
    # .h files
        network/acceptor.h
        network/client_handler.h
        network/client_sender.h
        network/server_protocol.h
        game/game.h
        game/game_manager.h
        game/game_types.h
        game/player_record.h
        inventory/inventory.h
        player/player.h
        entities/entity_id.h
        dungeon/dungeon_room.h
        dungeon/mineral.h
        dungeon/dungeon.h
        entities/entity_id_generator.h
        dungeon/dungeon_config.h
        player/player_config.h
        world/zone_map.h
)

target_include_directories(taller_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

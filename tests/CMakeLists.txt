target_sources(taller_tests
    PRIVATE
    # .cpp files
    network_test.cpp
    ../client/client_protocol.cpp
    ../server/network/acceptor.cpp
    ../server/network/client_handler.cpp
    ../server/network/client_sender.cpp
    ../server/network/server_protocol.cpp
    ../server/game/game.cpp
    ../server/game/game_manager.cpp
    ../server/player/player.cpp
    ../server/inventory/inventory.cpp
    ../server/dungeon/mineral.cpp
    ../server/dungeon/dungeon_room.cpp
    ../server/dungeon/dungeon.cpp
    ../server/entities/entity_id_generator.cpp
    ../server/world/zone_map.cpp
    PUBLIC
    # .h files
)

target_include_directories(taller_tests
    PUBLIC
    ../client
    ../server
)

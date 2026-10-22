target_sources(taller_tests
    PRIVATE
    # .cpp files
    network_test.cpp
    ../client/client_protocol.cpp
    ../server/acceptor.cpp
    ../server/client_handler.cpp
    ../server/client_sender.cpp
    ../server/server_protocol.cpp
    ../server/game.cpp
    ../server/game_manager.cpp
    PUBLIC
    # .h files
)

target_include_directories(taller_tests
    PUBLIC
    ../client
    ../server
)
target_sources(taller_server
    PRIVATE
    # .cpp files
        main.cpp
        acceptor.cpp
        client_handler.cpp
        server_protocol.cpp
        game.cpp
        game_manager.cpp

    PUBLIC
    # .h files
        acceptor.h
        client_handler.h
        server_protocol.h
        game.h
        game_manager.h
        game_types.h
        player_record.h
)
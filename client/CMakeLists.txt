target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    game.cpp
    PUBLIC
    # .h files
    game.h
    input_translator.h
    network.h
    recv_thread.h
    send_thread.h
    state.h
    )

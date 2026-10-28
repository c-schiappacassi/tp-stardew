target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    sdlGUI/world/player_view.cpp
    sdlGUI/asset_manager.cpp
    game.cpp
    PUBLIC
    # .h files
    game.h
    input_translator.h
    network.h
    recv_thread.h
    send_thread.h
    state.h
    sdlGUI/asset_manager.h
    sdlGUI/world/player_view.h
    sdlGUI/render_info.h
    sdlGUI/utils.h
    sdlGUI/animator.h
    sdlGUI/camera.h
    )

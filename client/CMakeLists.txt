target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    game.cpp
    game_ui.cpp
    sdlGUI/asset_manager.cpp

    sdlGUI/world/player_view.cpp
    sdlGUI/world/furniture_view.cpp

    sdlGUI/world/hud/hotbar_view.cpp
    sdlGUI/world/hud/bar_view.cpp

    sdlGUI/world/mines/ore_view.cpp
    sdlGUI/world/mines/level_view.cpp
    sdlGUI/world/mines/mine_view.cpp
    sdlGUI/world/mines/enemy_view.cpp
    sdlGUI/world/mines/trap_view.cpp
    sdlGUI/world/mines/projectile_view.cpp

    sdlGUI/world/farm/crop_view.cpp
    sdlGUI/world/farm/animal_view.cpp

    PUBLIC
    # .h files
    game.h
    game_ui.h
    network.h
    recv_thread.h
    send_thread.h
    state.h
    
    sdlGUI/asset_manager.h
    sdlGUI/render_info.h
    sdlGUI/utils.h
    sdlGUI/animator.h
    sdlGUI/camera.h
    sdlGUI/sync_views.h
    sdlGUI/draw_region.h
    sdlGUI/sprite_defs.h

    sdlGUI/ui/input_event.h
    sdlGUI/ui/sdl_input.h
    sdlGUI/ui/ui_layer.h
    sdlGUI/ui/ui_manager.h
    sdlGUI/ui/world_input.h
    sdlGUI/ui/layers/chat_layer.h
    sdlGUI/ui/layers/ready_to_sleep_layer.h
    sdlGUI/ui/layers/hud_layer.h

    sdlGUI/world/player_view.h
    sdlGUI/world/world_view.h
    sdlGUI/world/map_view.h
    sdlGUI/world/zone_view.h
    sdlGUI/world/entity_view.h
    sdlGUI/world/furniture_view.h
    sdlGUI/world/background.h
    sdlGUI/world/background_zone_view.h

    sdlGUI/world/interiors/interior_view.h
    sdlGUI/world/interiors/interiors_view.h

    sdlGUI/world/hud/hud_view.h
    sdlGUI/world/hud/chat_view.h
    sdlGUI/world/hud/hotbar_view.h
    sdlGUI/world/hud/bar_view.h
    sdlGUI/world/hud/ready_to_sleep_view.h
    sdlGUI/world/hud/pixel_font.h

    sdlGUI/world/mines/level_view.h
    sdlGUI/world/mines/mine_view.h
    sdlGUI/world/mines/ore_view.h
    sdlGUI/world/mines/enemy_view.h
    sdlGUI/world/mines/trap_view.h
    sdlGUI/world/mines/projectile_view.h

    sdlGUI/world/farm/farm_view.h
    sdlGUI/world/farm/crop_view.h
    sdlGUI/world/farm/animal_view.h
    )

add_subdirectory(qt)
target_include_directories(taller_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(taller_client PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/common/assets")

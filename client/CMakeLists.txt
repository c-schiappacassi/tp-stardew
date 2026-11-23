target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    game.cpp
    game_ui.cpp
    sdlGUI/asset_handling/asset_manager.cpp
    sdlGUI/asset_handling/asset_loader.cpp

    sdlGUI/world/background/furniture_view.cpp
    sdlGUI/world/background/item_view.cpp

    sdlGUI/world/hud/hotbar_view.cpp
    sdlGUI/world/hud/bar_view.cpp
    sdlGUI/world/hud/minimap_view.cpp

    sdlGUI/world/player/player_view.cpp

    sdlGUI/world/mines/ore_view.cpp
    sdlGUI/world/mines/level_view.cpp
    sdlGUI/world/mines/mine_view.cpp
    sdlGUI/world/mines/enemy_view.cpp
    sdlGUI/world/mines/trap_view.cpp
    sdlGUI/world/mines/projectile_view.cpp

    sdlGUI/world/farm/crop_view.cpp
    sdlGUI/world/farm/animal_view.cpp

    ${CMAKE_SOURCE_DIR}/common/map/map_shapes.cpp

    PUBLIC
    # .h files
    game.h
    game_ui.h
    network.h
    recv_thread.h
    send_thread.h
    state.h
    

    sdlGUI/render_info.h
    sdlGUI/utils.h
    sdlGUI/animator.h
    sdlGUI/clip_player.h
    sdlGUI/camera.h
    sdlGUI/sync_views.h
    sdlGUI/draw_region.h

    sdlGUI/asset_handling/sprite_defs.h
    sdlGUI/asset_handling/asset_manager.h
    sdlGUI/asset_handling/asset_loader.h
    sdlGUI/asset_handling/asset_tables.h

    sdlGUI/ui/input_event.h
    sdlGUI/ui/sdl_input.h
    sdlGUI/ui/ui_layer.h
    sdlGUI/ui/ui_manager.h
    sdlGUI/ui/world_input.h
    sdlGUI/ui/layers/modal_layer.h
    sdlGUI/ui/layers/chat_layer.h
    sdlGUI/ui/layers/ready_to_sleep_layer.h
    sdlGUI/ui/layers/hud_layer.h
    sdlGUI/ui/layers/quit_layer.h
    sdlGUI/ui/layers/minimap_layer.h
    sdlGUI/ui/layers/night_sky_layer.h

    sdlGUI/world/lighting.h
    sdlGUI/world/world_view.h
    sdlGUI/world/map_view.h
    sdlGUI/world/zone_view.h
    sdlGUI/world/entity_view.h

    sdlGUI/world/effects/health_trigger.h
    sdlGUI/world/effects/hit_flash.h

    sdlGUI/world/background/item_view.h
    sdlGUI/world/background/furniture_view.h
    sdlGUI/world/background/background.h
    sdlGUI/world/background/background_zone_view.h

    sdlGUI/world/player/player_view.h

    sdlGUI/world/interiors/interior_view.h
    sdlGUI/world/interiors/interiors_view.h

    sdlGUI/world/hud/hud_view.h
    sdlGUI/world/hud/dialog_view.h
    sdlGUI/world/hud/minimap_view.h
    sdlGUI/world/hud/chat_view.h
    sdlGUI/world/hud/hotbar_view.h
    sdlGUI/world/hud/bar_view.h
    sdlGUI/world/hud/ready_to_sleep_view.h
    sdlGUI/world/hud/night_sky_view.h
    sdlGUI/world/hud/quit_view.h
    sdlGUI/world/hud/pixel_font.h
    sdlGUI/world/hud/chat_pixel_font.h

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

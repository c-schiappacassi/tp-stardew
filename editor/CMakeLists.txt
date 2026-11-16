target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    main_window.cpp
    map_editor_screen.cpp
    sprite_button.cpp
    start_screen.cpp
    model/catalog.cpp
    zone/side_bar.cpp
    zone/sprites.cpp
    zone/zone_tab.cpp
    model/world_map.cpp
    model/zone_map.cpp
    zone/zone_canvas.cpp
    PUBLIC
    # .h files
    editor_constants.h
    main_window.h
    map_editor_screen.h
    sprite_button.h
    start_screen.h
    model/catalog.h
    zone/side_bar.h
    zone/sprites.h
    zone/zone_tab.h
    model/world_map.h
    model/zone_map.h
    zone/tool.h
    zone/zone_canvas.h
    )

target_include_directories(taller_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(taller_editor PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/common/assets")

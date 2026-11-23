cmake_minimum_required(VERSION 3.24)

# Also accept the former Qt-only build entry point.
if(NOT TARGET taller_client)
    project(stardew_client_qt LANGUAGES C CXX)
    set(TALLER_SERVER OFF CACHE BOOL "" FORCE)
    set(TALLER_EDITOR OFF CACHE BOOL "" FORCE)
    set(TALLER_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(../.. stardew)
    add_custom_target(stardew_client_qt DEPENDS taller_client)
    return()
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets GLOBAL)
set_target_properties(taller_client PROPERTIES AUTOMOC ON)

target_sources(taller_client PRIVATE
    main_window.h main_window.cpp
    username_screen.h username_screen.cpp
    start_menu_screen.h start_menu_screen.cpp
    farmer_sprites.h farmer_sprites.cpp
    character_screen.h character_screen.cpp
    new_game_screen.h new_game_screen.cpp
    load_screen.h load_screen.cpp
    join_screen.h join_screen.cpp
    join_character_screen.h join_character_screen.cpp
    sprite_button.h sprite_button.cpp
    menu_helpers.h menu_helpers.cpp
    map_selector.h map_selector.cpp
)
target_link_libraries(taller_client PRIVATE Qt6::Core Qt6::Widgets)

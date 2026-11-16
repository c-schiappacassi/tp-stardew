target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    main_window.cpp
    map_editor_screen.cpp
    sprite_button.cpp
    start_screen.cpp
    PUBLIC
    # .h files
    editor_constants.h
    main_window.h
    map_editor_screen.h
    sprite_button.h
    start_screen.h
    )

target_compile_definitions(taller_editor PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/common/assets")

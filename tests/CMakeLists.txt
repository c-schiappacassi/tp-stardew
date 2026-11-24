target_sources(taller_tests
    PRIVATE
    # .cpp files

    PUBLIC
    # .h files
)

target_include_directories(taller_tests
    PUBLIC
    ../client
    ../server
)

target_compile_definitions(taller_tests PRIVATE WORLD_EXAMPLE_PATH="${PROJECT_SOURCE_DIR}/server/config/world.toml")

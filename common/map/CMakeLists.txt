target_sources(taller_map
    PRIVATE
    # .cpp files
        map_loader.cpp
        map_rules.cpp
        map_shapes.cpp
        map_writer.cpp

    PUBLIC
    # .h files
        map_data.h
        map_loader.h
        map_rules.h
        map_shapes.h
        map_writer.h
)

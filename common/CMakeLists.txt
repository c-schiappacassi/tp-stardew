target_sources(taller_common
    PRIVATE
    # .cpp files
        liberror.cpp
        protocol_base.cpp
        protocol_errors.cpp
        resolver.cpp
        resolvererror.cpp
        socket.cpp
        message_buffer.cpp

    PUBLIC
    # .h files
        message_buffer.h
        constants.h
        dtos.h
        furniture_data.h
        game_time.h
        foo.h
        position.h
        foo.tpp
        liberror.h
        protocol_base.h
        protocol_errors.h
        protocol_types.h
        queue.h
        resolver.h
        resolvererror.h
        socket.h
        thread.h
)

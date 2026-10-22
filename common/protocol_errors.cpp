#include "protocol_errors.h"

#include <string>

ConnectionClosed::ConnectionClosed()
    : std::runtime_error("se cerro la conexion") {}

ProtocolError::ProtocolError(const std::string& what)
    : std::runtime_error(what) {}

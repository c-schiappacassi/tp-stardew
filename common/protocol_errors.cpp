#include "protocol_errors.h"

#include <string>

ConnectionClosed::ConnectionClosed()
    : std::runtime_error("connection closed") {}

ProtocolError::ProtocolError(const std::string& what)
    : std::runtime_error(what) {}

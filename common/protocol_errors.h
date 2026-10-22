#ifndef PROTOCOL_ERRORS_H
#define PROTOCOL_ERRORS_H

#include <stdexcept>
#include <string>

class ConnectionClosed : public std::runtime_error {
public:
    ConnectionClosed();
};

class ProtocolError : public std::runtime_error {
public:
    explicit ProtocolError(const std::string& what);
};

#endif

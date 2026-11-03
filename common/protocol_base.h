#pragma once

#include <cstdint>
#include <string>

#include "socket.h"


class ProtocolBase {
private:
    Socket& socket;

    void sendBytes(const void* data, unsigned int size);
    void receiveBytes(void* data, unsigned int size);

public:
    explicit ProtocolBase(Socket& socket);

    ProtocolBase(const ProtocolBase&) = delete;
    ProtocolBase& operator=(const ProtocolBase&) = delete;

    void sendUint8(const uint8_t value);
    void sendUint16(const uint16_t value);
    void sendUint32(const uint32_t value);
    void sendString(const std::string& value);

    uint8_t receiveUint8();
    uint16_t receiveUint16();
    uint32_t receiveUint32();
    std::string receiveString();
};

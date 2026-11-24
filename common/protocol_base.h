#pragma once

#include <cstdint>
#include <string>

#include "message_buffer.h"
#include "socket.h"


class ProtocolBase {
private:
    Socket socket;

    void receiveBytes(void* data, unsigned int size);

public:
    explicit ProtocolBase(Socket&& socket);

    ProtocolBase(const ProtocolBase&) = delete;
    ProtocolBase& operator=(const ProtocolBase&) = delete;
    ProtocolBase(ProtocolBase&&) = default;
    ProtocolBase& operator=(ProtocolBase&&) = default;

    void send(const MessageBuffer& message);
    void shutdown();

    uint8_t receiveUint8();
    uint16_t receiveUint16();
    uint32_t receiveUint32();
    std::string receiveString();
};

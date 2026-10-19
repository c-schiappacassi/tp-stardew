#include "protocol_base.h"

#include <arpa/inet.h>

#include <cstdint>
#include <limits>
#include <string>

#include "protocol_errors.h"

ProtocolBase::ProtocolBase(Socket& socket) : socket(socket) {}

void ProtocolBase::sendBytes(const void* data, unsigned int size) {
    if (socket.sendall(data, size) == 0) {
        throw ConnectionClosed();
    }
}

void ProtocolBase::receiveBytes(void* data, unsigned int size) {
    if (socket.recvall(data, size) == 0) {
        throw ConnectionClosed();
    }
}


void ProtocolBase::sendUint8(const uint8_t value) {
    sendBytes(&value, sizeof(value));
}

void ProtocolBase::sendUint16(const uint16_t value) {
    const uint16_t send = htons(value);
    sendBytes(&send, sizeof(send));
}

void ProtocolBase::sendUint32(const uint32_t value) {
    const uint32_t send = htonl(value);
    sendBytes(&send, sizeof(send));
}

void ProtocolBase::sendString(const std::string& value) {
    if (value.size() > std::numeric_limits<uint16_t>::max()) {
        throw ProtocolError("string demasiado largo");
    }
    sendUint16(static_cast<uint16_t>(value.size()));
    if (!value.empty()) {
        sendBytes(value.data(), static_cast<unsigned int>(value.size()));
    }
}

uint8_t ProtocolBase::receiveUint8() {
    uint8_t value = 0;
    receiveBytes(&value, sizeof(value));
    return value;
}

uint16_t ProtocolBase::receiveUint16() {
    uint16_t value = 0;
    receiveBytes(&value, sizeof(value));
    return ntohs(value);
}

uint32_t ProtocolBase::receiveUint32() {
    uint32_t value = 0;
    receiveBytes(&value, sizeof(value));
    return ntohl(value);
}

std::string ProtocolBase::receiveString() {
    const uint16_t length = receiveUint16();
    std::string value(length, '\0');
    if (length > 0) {
        receiveBytes(value.data(), length);
    }
    return value;
}

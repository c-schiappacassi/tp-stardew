#include "protocol_base.h"

#include <arpa/inet.h>

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "protocol_errors.h"


ProtocolBase::ProtocolBase(Socket&& socket) : socket(std::move(socket)) {}

void ProtocolBase::send(const MessageBuffer& message) {
    if (message.size() == 0) {
        return;
    }
    if (message.size() > std::numeric_limits<unsigned int>::max()) {
        throw ProtocolError("mensaje demasiado largo");
    }
    if (socket.sendall(message.bytes(), static_cast<unsigned int>(message.size())) == 0) {
        throw ConnectionClosed();
    }
}

void ProtocolBase::shutdown(int how) { socket.shutdown(how); }

void ProtocolBase::receiveBytes(void* data, unsigned int size) {
    if (socket.recvall(data, size) == 0) {
        throw ConnectionClosed();
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

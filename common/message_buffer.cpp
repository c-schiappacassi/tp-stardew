#include "message_buffer.h"

#include <limits>

#include "protocol_errors.h"

MessageBuffer::MessageBuffer(std::size_t reserveBytes) { data.reserve(reserveBytes); }

void MessageBuffer::putUint8(const uint8_t value) { data.push_back(value); }

void MessageBuffer::putUint16(const uint16_t value) {
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value & 0xFF));
}

void MessageBuffer::putUint32(const uint32_t value) {
    data.push_back(static_cast<uint8_t>(value >> 24));
    data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(value & 0xFF));
}

void MessageBuffer::putString(const std::string& value) {
    if (value.size() > std::numeric_limits<uint16_t>::max()) {
        throw ProtocolError("string demasiado largo");
    }
    putUint16(static_cast<uint16_t>(value.size()));
    data.insert(data.end(), value.begin(), value.end());
}

const uint8_t* MessageBuffer::bytes() const { return data.data(); }

std::size_t MessageBuffer::size() const { return data.size(); }

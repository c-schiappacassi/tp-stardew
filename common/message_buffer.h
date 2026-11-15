#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


// armamos UN mensaje completo en memoria y asi hacemos un unico send ( big endian )

class MessageBuffer {
private:
    std::vector<uint8_t> data;

public:
    explicit MessageBuffer(std::size_t reserveBytes = 64);

    void putUint8(uint8_t value);
    void putUint16(uint16_t value);
    void putUint32(uint32_t value);
    void putString(const std::string& value);

    const uint8_t* bytes() const;
    std::size_t size() const;
};

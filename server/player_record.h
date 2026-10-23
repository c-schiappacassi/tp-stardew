#ifndef PLAYER_RECORD_H
#define PLAYER_RECORD_H

#include <cstdint>
#include <memory>
#include <string>

#include "game_types.h"
#include "position.h"

struct PlayerRecord {
    uint16_t id;
    std::string name;
    uint8_t skin;
    std::shared_ptr<OutQueue> output;
    bool connected;
    Position position{0, 0};
};

#endif
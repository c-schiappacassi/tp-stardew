#ifndef PLAYER_RECORD_H
#define PLAYER_RECORD_H

#include <cstdint>
#include <memory>
#include <string>

#include "game/game_types.h"
#include "player/player.h"

struct PlayerRecord {
    uint16_t id;
    std::string name;
    std::shared_ptr<OutQueue> output;
    bool connected;
    Player player;
};

#endif
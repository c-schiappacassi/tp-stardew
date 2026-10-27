#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <cstdint>
#include <memory>
#include <variant>
#include <string>

#include "dtos.h"
#include "queue.h"


struct PlayerCommand {
    std::string playerName;
    std::shared_ptr<CommandDTO> command;
};

// esto despues lo sacamos pero es para no romper nada de lo que hicieron
using OutgoingMessage = ServerStatus;

// lo de abajo es como establecer un "sinonimo" y uso ese nombre en todo el codigo
using CommandQueue = Queue<PlayerCommand>;
using OutQueue = Queue<OutgoingMessage>;

struct GameSession {
    ServerStatus status;
    std::string gameId;
    std::string playerName;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    std::shared_ptr<CommandQueue> input;   // nullptr si status != OK
};

#endif
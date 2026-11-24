#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "dtos.h"
#include "queue.h"

struct PlayerCommand {
    uint16_t playerId = 0;
    std::shared_ptr<CommandDTO> command;
};

using OutQueue = Queue<ServerStatus>;

enum class CommandSubmission {
    Enqueued,
    GameNotFound,
    GameClosed
};

struct GameSession {
    ServerStatus status;
    std::string gameId;
    std::string userName;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    std::shared_ptr<const ConstMapData> map;
};

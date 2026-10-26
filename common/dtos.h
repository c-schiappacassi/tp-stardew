#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "protocol_types.h"
#include "position.h"


struct CreateGameDTO {
    std::string gameName;
    std::string playerName;
    uint8_t skin;
    uint8_t worldId;
};

struct JoinGameDTO {
    std::string code;
    std::string playerName;
    uint8_t skin;
};

struct ResumeGameDTO {
    std::string gameId;
};

struct GameInfoDTO {
    std::string id;
    std::string gameName;
    std::string playerName;
    uint8_t skin;
    bool isMultiplayer;
};

struct GameListDTO {
    std::vector<GameInfoDTO> games;
};

struct GameJoinedDTO {
    std::string gameId;
    uint16_t playerId;
    uint8_t worldId;
};

struct StatusDTO {
    StatusCode status;
    std::string errorMsg;
    std::string successMsg;
};

struct MoveDTO {
    Position position{0, 0};
};

// TODO: seguir completando server status
struct ServerStatus {
    StatusCode status;
    std::string errorMsg;
    std::string successMsg;
    std::string gameId;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    Position positionPlayer{0, 0};
};

using GameDTO = std::variant<CreateGameDTO, JoinGameDTO, MoveDTO>;

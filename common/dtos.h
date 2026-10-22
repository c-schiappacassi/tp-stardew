#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "protocol_types.h"


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

struct ServerStatus
{
    // TODO
};

using GameDTO = std::variant<CreateGameDTO, JoinGameDTO>;

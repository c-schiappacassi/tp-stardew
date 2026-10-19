#ifndef DTOS_H
#define DTOS_H

#include <cstdint>
#include <string>
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
    uint16_t gameId;
};

struct GameInfoDTO {
    uint16_t id;
    std::string gameName;
    std::string playerName;
    uint8_t skin;
    bool isMultiplayer;
};

struct GameListDTO {
    std::vector<GameInfoDTO> games;
};

struct GameJoinedDTO {
    uint16_t playerId;
    uint8_t worldId;
};

#endif

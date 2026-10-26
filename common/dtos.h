#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "protocol_types.h"
#include "position.h"

// TODO IMPORTANTE: cambio de variant a herencia porque todos los dtos mandan el id del usuario en la 
// partida para que la partida identifique a que clientHandler le tiene que responder
// TBD: con que identificar al usuario(el apodo de la partida o un id?)

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

struct PlayerData
{
    /* data */
};

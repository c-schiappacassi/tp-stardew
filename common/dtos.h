#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

#include "protocol_types.h"
#include "position.h"


// TODO IMPORTANTE: cambio de variant a herencia porque todos los dtos mandan el id del usuario en la
// partida para que la partida identifique a que clientHandler le tiene que responder
// TBD: con que identificar al usuario(el apodo de la partida o un id?)

struct CommandDTO {
    std::string playerName;

    explicit CommandDTO(std::string name) : playerName(std::move(name)) {
        if (playerName.empty()) {
            throw std::invalid_argument("playerName no puede estar vacio");
        }
    }
    virtual ~CommandDTO() = default;
};

// CLIENTE -> SERVIDOR

struct CreateGameDTO : public CommandDTO {
    std::string gameName;
    uint8_t skin;
    uint8_t worldId;

    explicit CreateGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct JoinGameDTO : public CommandDTO {
    std::string code;
    uint8_t skin;

    explicit JoinGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct ResumeGameDTO : public CommandDTO {
    uint16_t gameId;

    explicit ResumeGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct MoveDTO : public CommandDTO {
    Position position{0, 0};

    explicit MoveDTO(const std::string& name) : CommandDTO(name) {}
};

struct InteractDTO : public CommandDTO {
    TargetType targetType; // usemos un type para esto
    uint16_t targetId;

    explicit InteractDTO(const std::string& name) : CommandDTO(name) {}
};

struct DonateDTO : public CommandDTO {
    uint8_t slot;
    uint32_t quantity;

    explicit DonateDTO(const std::string& name) : CommandDTO(name) {}
};

struct ChatDTO : public CommandDTO {
    std::string text;

    explicit ChatDTO(const std::string& name) : CommandDTO(name) {}
};

struct UseItemDTO : public CommandDTO {
    uint8_t itemSlot{0};

    explicit UseItemDTO(const std::string& name) : CommandDTO(name) {}
};

struct RequestMultiplayerCodeDTO : public CommandDTO {
    explicit RequestMultiplayerCodeDTO(const std::string& name) : CommandDTO(name) {}
};

struct DisconnectDTO : public CommandDTO {
    explicit DisconnectDTO(const std::string& name) : CommandDTO(name) {}
};

// DEBERIAMOS VOLARLOS:

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

// SERVIDOR -> CLIENTE

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

struct GameInfoDTO {
    uint16_t id;
    std::string gameName;
    std::string playerName;
    uint8_t skin;
    bool isMultiplayer; // // tal vez false deberia ser el default bool isMultiplayer{false};
};

struct PlayerData
{
    /* data */
};

/*
// aca dejo un compilado de los dtos que ya habia estado viendo antes para completar el serverstatus

struct ServerStatus {
    uint16_t gameId;
    uint8_t worldId;

    StatusCode status{};
    std::string errorMsg;
    std::string successMsg;

    bool isFinished; // tal vez false deberia ser el default bool isFinished{false};

    std::string MultiplayerCode;
    std::vector<GameInfoDTO> currentGames; // ojo que aca pasamos vector de DTOs

    // y esta es una idea para pasar todos los datos en vectores donde la posicion siempre corresponde a un mismo jugador
    std::vector<uint16_t> playerIds;
    std::vector<std::string> playerNames;
    std::vector<uint8_t> playerSkins;
    std::vector<Position> playerPositions;
    std::vector<uint16_t> playerHealths;
    std::vector<uint32_t> playerMoney;

    // y que el chat sea emisor + mensaje pero creo que ciro necesitaria tambien un receivers por los mensajes privados
    // la otra es usar lo mismo que arriba y hacer indice = jugador y se va pasando de a un solo mensaje (creo que esto es mas prolijo para leerlo despues porque sigue la misma logica que los vectores de arriba)
    std::vector<std::string> chatSenders;
    std::vector<std::string> chatTexts;
};
*/

#endif // DTOS_H

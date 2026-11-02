#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <map>

#include "protocol_types.h"
#include "position.h"


// TODO IMPORTANTE: cambio de variant a herencia porque todos los dtos mandan el id del usuario en la
// partida para que la partida identifique a que clientHandler le tiene que responder
// TBD: con que identificar al usuario(el apodo de la partida o un id?)

// AUXILIARES

struct Color{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct PlayerData{
    std::string characterName;
    Position pos{0,0};
    PlayerState state{PlayerState::Idle};
    Direction dir{Direction::None};
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0,0,0};
    Color pantsColor{0,0,0};
};

// COMMAND_DTO CON HERENCIA

struct CommandDTO {
    std::string userName;

    explicit CommandDTO(std::string name) : userName(std::move(name)) {
        if (userName.empty()) {
            throw std::invalid_argument("userName no puede estar vacio");
        }
    }
    virtual ~CommandDTO() = default;
    virtual MessageType type() const = 0;
};

// CLIENTE -> SERVIDOR

struct CreateGameDTO : public CommandDTO {
    std::string characterName;
    std::string gameName;
    uint8_t worldId;
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0,0,0};
    Color pantsColor{0,0,0};

    MessageType type() const override { return MessageType::CreateGame; }
    explicit CreateGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct JoinGameDTO : public CommandDTO {
    std::string characterName;
    std::string code;
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0,0,0};
    Color pantsColor{0,0,0};

    MessageType type() const override { return MessageType::JoinGame; }
    explicit JoinGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct ResumeGameDTO : public CommandDTO {
    uint16_t gameId;

    MessageType type() const override { return MessageType::ResumeGame; }
    explicit ResumeGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct MoveDTO : public CommandDTO {
    Position position{0, 0};
    Direction dir{Direction::None};

    MessageType type() const override { return MessageType::Move; }
    explicit MoveDTO(const std::string& name) : CommandDTO(name) {}
};

struct InteractDTO : public CommandDTO {
    TargetType targetType; // usemos un type para esto
    uint16_t targetId;

    MessageType type() const override { return MessageType::Interact; }
    explicit InteractDTO(const std::string& name) : CommandDTO(name) {}
};

struct DonateDTO : public CommandDTO {
    uint8_t slot;
    uint32_t quantity;

    MessageType type() const override { return MessageType::Donate; }
    explicit DonateDTO(const std::string& name) : CommandDTO(name) {}
};

struct ChatDTO : public CommandDTO {
    std::string text;

    MessageType type() const override { return MessageType::Chat; }
    explicit ChatDTO(const std::string& name) : CommandDTO(name) {}
};

struct UseItemDTO : public CommandDTO {
    uint8_t itemSlot;

    MessageType type() const override { return MessageType::UseItem; }
    explicit UseItemDTO(const std::string& name) : CommandDTO(name) {}
};

struct RequestMultiplayerCodeDTO : public CommandDTO {

    MessageType type() const override { return MessageType::MultiplayerCode; }
    explicit RequestMultiplayerCodeDTO(const std::string& name) : CommandDTO(name) {}
};

struct DisconnectDTO : public CommandDTO {

    MessageType type() const override { return MessageType::ExitGame; }
    explicit DisconnectDTO(const std::string& name) : CommandDTO(name) {}
};

struct RequestGameListDTO : public CommandDTO {

    MessageType type() const override { return MessageType::ListGames; }
    explicit RequestGameListDTO(const std::string& name) : CommandDTO(name) {}
};


// DEBERIAMOS VOLARLOS:
/*
 *
// SERVIDOR -> CLIENTE

// TODO: seguir completando server status
struct ServerStatus {
    StatusCode status;
    std::string errorMsg;
    std::string successMsg;
    std::string gameId;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    std::map<std::string, PlayerData> players;
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
    std::vector<std::string> userNames;
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

// SERVIDOR --> CLIENTE
// habria que ver si esto por ser qt se tiene que enviar "suelto" o dentro del server status"

struct GameInfoDTO {
    uint16_t id = 0;
    std::string farmName;
    PlayerData playerData;
    bool isMultiplayer = false;
};

struct GameListDTO {
    std::vector<GameInfoDTO> games;
};

// hay que agregar abajo del wordId: (serverstatus)
// std::vector<GameInfoDTO> games;

#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <unordered_map>
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

struct Stack {
    ItemId item_id;
    uint16_t quantity;
    Quality qual; 
};

struct PlayerData{
    Position pos{0,0};
    PlayerState state{PlayerState::Idle};
    Direction dir{Direction::None};
    Zone zone{Zone::Farm};
    uint8_t dungeonLevel = 0;  // 0 seria afuera de la mazmorra por ahora )?
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0, 0, 0};
    Color pantsColor{0, 0, 0};
};

struct PlayerDataLocal : public PlayerData {
    std::map<uint8_t, Stack> items;
    uint8_t current_slot = 0;
    uint32_t gold = 0;
    uint16_t health = 0;
    uint16_t maxHealth = 0;
    uint16_t energy = 0;
    uint16_t maxEnergy = 0;

    PlayerDataLocal() = default;
    explicit PlayerDataLocal(const PlayerData& publicData): PlayerData(publicData) {}
};

struct MineralData {
    uint32_t id = 0;
    Position pos{0, 0};
    ResourceType type = ResourceType::CoalRock;
    MineralState state = MineralState::Intact;
};

// COMMAND_DTO CON HERENCIA

struct CommandDTO {
    std::string playerName;

    explicit CommandDTO(std::string name) : playerName(std::move(name)) {
        if (playerName.empty()) {
            throw std::invalid_argument("playerName no puede estar vacio");
        }
    }
    virtual ~CommandDTO() = default;
    virtual MessageType type() const = 0;
};

// CLIENTE -> SERVIDOR

struct CreateGameDTO : public CommandDTO {
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
    uint32_t targetId;

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

struct SelectSlotDTO : public CommandDTO {
    uint8_t slot = 0;

    MessageType type() const override { return MessageType::SelectSlot; }
    explicit SelectSlotDTO(const std::string& name) : CommandDTO(name) {}
};

// SERVIDOR -> CLIENTE

// seguir completando server status
struct ServerStatus {
    StatusCode status;
    std::string errorMsg;
    std::string successMsg;
    std::string gameId;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    std::map<std::string, PlayerData> players;
    PlayerDataLocal localPlayer;
    std::optional<MineData> mine;
};

struct LevelData {
    Position entry{0, 0};
    Position exit{0, 0};
    std::optional<Position> nextRoom;
    std::vector<MineralData> minerals;
};

struct MineData {
    uint8_t current_level;
    LevelData current_level_data;
};


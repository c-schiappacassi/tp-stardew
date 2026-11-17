#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "position.h"
#include "protocol_types.h"
#include "game_time.h"

// AUXILIARES

// Farm
struct CropData{
    uint32_t id = 0;
    Position pos{0,0};
    CropType type = CropType::Strawberry;
    CropState state = CropState::JustPlanted;
    bool wattered = false;
};

struct AnimalData{
    uint32_t id = 0;
    Position pos{0,0};
    Direction dir{Direction::None};
    AnimalType type = AnimalType::BrownCow;
    AnimalState state = AnimalState::Idle;
    bool needs_interaction=false;
    // Age ? TBD si hacemos logica de que los animales llegan bebes y crecen
};

struct FurnitureData {
    uint32_t id = 0;
    Position pos{0, 0};
    FurnitureId type = FurnitureId::Chair;
    Direction dir = Direction::Down;
};

struct FarmData{
    std::vector<CropData> crops;
    std::vector<AnimalData> animals;
    std::vector<Position> tilledSoil;
    std::vector<FurnitureData> furniture;
};

struct InteriorData {
    InteriorId interior_id = InteriorId::PlayerHouse;
    uint8_t house_index = 0;
    std::vector<FurnitureData> furniture;
};

// FIN Farm

// Mine
struct EnemyData{
    uint32_t id = 0;
    Position pos{0,0};
    Direction dir{Direction::None};
    EnemyType type=EnemyType::Skeleton;
    EnemyState state=EnemyState::Idle;
};

struct EnemySnapshot: EnemyData {
    uint16_t health = 0;
    uint16_t maxHealth = 0;
    bool frenzied = false;
    uint32_t action = 0;
};

struct ProjectileData {
    uint32_t id = 0;
    Position pos{0, 0};
    FlameType type = FlameType::Blue;
};

struct TrapData {
    uint32_t id = 0;
    Position pos{0, 0};
    Direction dir = Direction::Down;
    TrapType type = TrapType::Spikes;
    TrapState state = TrapState::Inactive;
    uint16_t range = 0;
};

struct LootData {
    uint32_t id = 0;
    Position pos{0, 0};
    ItemId item = ItemId::Silver;
    uint16_t quantity = 1;
};

struct CombatProgressData {
    bool skeletons = false;
    bool vampires = false;
    bool orcs = false;
    bool skull = false;
};

struct DungeonCombatData {
    uint32_t visitId = 0;
    uint8_t room = 0;
    std::vector<EnemySnapshot> enemies;
    std::vector<ProjectileData> projectiles;
    std::vector<TrapData> traps;
    std::vector<LootData> loot;
};

struct MineralData {
    uint32_t id = 0;
    Position pos{0, 0};
    ResourceType type = ResourceType::CoalRock;
    MineralState state = MineralState::Intact;
};

struct LevelData {
    Position entry{0, 0};
    Position exit{0, 0};
    std::vector<MineralData> minerals;
    DungeonCombatData combat;
};

struct MineData{
    uint8_t current_level = 0;
    LevelData current_level_data;
};
// Fin Mine

using ZoneData = std::variant<MineData, FarmData, InteriorData, std::monostate>;

// Map DTOS para el segundo handshake

// Constante: viaja en el segundo handshake, dentro de BackgroundData
struct TileData {
    Position pos{0,0};
    TileTextureId id = TileTextureId::Grass;
};

struct StaticFurnitureData {  // para la deco del pueblo o las casas de npcs
    Position pos{0,0};
    FurnitureId id = FurnitureId::Chair;
    Direction dir = Direction::None;
};

struct DecorData { // para cosas como arboles y plantas
    Position pos{0,0};
    DecorId id = DecorId::MapleTree;
};

struct BuildingData {
    Position pos{0,0};
    BuildingId id = BuildingId::FarmHouse;
};

struct PortalData {
    Position pos{0, 0};
    PortalId id = PortalId::FarmToMine;
};

struct NpcData{
    Position pos{0,0};
    NpcId id = NpcId::Marnie;
};

struct InteriorBackgroundData
{
    Position pos{0,0};
    InteriorId interior_id = InteriorId::PlayerHouse;
    std::optional<NpcData> npc;
};

struct BackgroundData{
    std::vector<TileData> floor;
    std::vector<DecorData> decor;
    std::vector<BuildingData> buildings; // Exteriores, para los interiores son una zona distinta, asi que les llega su propio BackgroundData
    std::vector<StaticFurnitureData> furniture;
    std::vector<PortalData> portals;
    std::optional<InteriorBackgroundData> interior;
};

// TODO: TBD primer draft del segundo Handshake, para la data que NUNCA cambia.
// se excluye:  Furniture (en casa del player cambia de lugar, pero en los shops de los npcs no.
//              el servidor se encarga de no permitieme interactuar con muebles de npcs)

struct ConstMapData{
    BackgroundData farm;
    std::vector<BackgroundData> levels;
    std::vector<BackgroundData> farm_houses;
    BackgroundData pierres;
    BackgroundData robins;
    BackgroundData clints;
    BackgroundData marnies;
    BackgroundData community_center;
    BackgroundData town;
};

// Player
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

struct InteriorLocation {
    InteriorId interior_id = InteriorId::PlayerHouse;
    uint8_t house_index = 0;

    bool operator==(const InteriorLocation&) const = default;
};

struct PlayerData{
    std::string characterName;
    Position pos{0,0};
    PlayerState state{PlayerState::Idle};
    Direction dir{Direction::None};
    uint16_t hairModel = 0;
    uint16_t shirtModel = 0;
    uint16_t pantsModel = 0;
    Color hairColor{0, 0, 0};
    Color pantsColor{0, 0, 0};
    Zone currentZone = Zone::Farm;
    std::optional<InteriorLocation> interior;
    std::optional<ItemId> heldItem;
};

struct GameInfoDTO {
    std::string id;
    std::string farmName;
    PlayerData playerData;
    bool isMultiplayer = false;
};

struct ChatMessageData {
    std::string sender;
    std::string text;
};

struct PlayerDataLocal : public PlayerData {
    std::map<uint8_t, Stack> items;
    uint8_t current_slot = 0;
    uint32_t gold = 0;
    uint16_t health = 0;
    uint16_t maxHealth = 0;
    uint16_t energy = 0;
    uint16_t maxEnergy = 0;
    ZoneData zoneData{std::monostate{}};
    CombatProgressData combatProgress;
    std::vector<ChatMessageData> newMessages;

    PlayerDataLocal() = default;
    explicit PlayerDataLocal(const PlayerData& publicData): PlayerData(publicData) {}
};
// FIN Player


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
    uint8_t worldId = 0;
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
    std::string gameId;

    MessageType type() const override { return MessageType::ResumeGame; }
    explicit ResumeGameDTO(const std::string& name) : CommandDTO(name) {}
};

struct MoveDTO : public CommandDTO {
    Direction dir{Direction::None};

    MessageType type() const override { return MessageType::Move; }
    explicit MoveDTO(const std::string& name): CommandDTO(name) {}
};

struct InteractDTO: public CommandDTO {
    Direction dir{Direction::None};

    MessageType type() const override { return MessageType::Interact; }
    explicit InteractDTO(const std::string& name) : CommandDTO(name) {}
};

struct DayOkDTO: public CommandDTO {
    explicit DayOkDTO(const std::string& name): CommandDTO(name) {}
    MessageType type() const override { return MessageType::DayOk; }
};

struct DayCancelDTO: public CommandDTO {
    explicit DayCancelDTO(const std::string& name): CommandDTO(name) {}
    MessageType type() const override { return MessageType::DayCancel; }
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

struct ListGamesDTO : public CommandDTO {

    MessageType type() const override { return MessageType::ListGames; }
    explicit ListGamesDTO(const std::string& name) : CommandDTO(name) {}
};

struct CheckCodeDTO : public CommandDTO {
    std::string code;

    MessageType type() const override { return MessageType::CheckCode; }
    explicit CheckCodeDTO(const std::string& name) : CommandDTO(name) {}
};

// SERVIDOR -> CLIENTE

// seguir completando server status



struct ServerStatus {
    StatusCode status = StatusCode::Ok;
    std::string errorMsg;
    std::string successMsg;
    std::string gameId;
    uint16_t playerId = 0;
    uint8_t worldId = 0;
    std::map<std::string, PlayerData> players;
    PlayerDataLocal localPlayer;
    bool dayEnded = false;
    bool dayConfirmationRequired = false;
    uint16_t timeOfDayMinutes = game_time::START_MINUTES;
    uint8_t cap_inventory; // Para la hotbar, con la recompensa del centro comunitario pasa de 9 -> 10
};

struct qtStatus {
    StatusCode status = StatusCode::Ok;
    std::string errorMsg;
    std::string successMsg;
    std::vector<GameInfoDTO> games;
};

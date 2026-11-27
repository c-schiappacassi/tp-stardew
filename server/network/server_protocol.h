#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <cstddef>
#include <limits>

#include "dtos.h"
#include "protocol_base.h"
#include "protocol_types.h"
#include "socket.h"


class ServerProtocol {
private:
    static constexpr size_t MAX_U8 = std::numeric_limits<uint8_t>::max();
    static constexpr size_t MAX_U16 = std::numeric_limits<uint16_t>::max();

    static constexpr uint8_t BUNDLE_COUNT = 5;
    static constexpr uint8_t REQUIREMENT_LIMIT = 10;
    static constexpr uint8_t SEED_COUNT = 4;

    static void requireCommunity(bool condition, const char* message);
    static void validateCommunityQuality(Quality quality);
    static void validateCommunityStacks(const std::vector<Stack>& items);
    static void validateDonationKind(DonationKind kind);
    static void validateCommunityState(const CommunityStateData& state);
    static void validateCount(size_t count, size_t limit, const char* what);
    static void validateFarmCounts(const FarmData& farm);
    static void validatePosition(const Position& pos);
    static void validateOpenChest(const OpenChestData& chest, const ZoneData& zone);
    static void validateLoot(const std::vector<LootData>& loot);
    static void validateCombat(const DungeonCombatData& combat);
    static bool hasMatchingZoneData(const PlayerDataLocal& data);
    static void validateStatus(const ServerStatus& status);

    ProtocolBase base;

    Position receivePosition();
    void writePosition(MessageBuffer& out, const Position& pos);

    Color receiveColor();
    void writeColor(MessageBuffer& out, const Color& color);

    Direction receiveDirection();

    void writeStack(MessageBuffer& out, const Stack& stack);
    void writeOpenChest(MessageBuffer& out, const std::optional<OpenChestData>& chest);
    void writeCommunityStacks(MessageBuffer& out, const std::vector<Stack>& items);
    void writeCommunityState(MessageBuffer& out, const CommunityStateData& state);
    void writeMineralData(MessageBuffer& out, const MineralData& mineral);
    void writeLootData(MessageBuffer& out, const std::vector<LootData>& loot);
    void writeCombatData(MessageBuffer& out, const DungeonCombatData& combat);
    void writeLevelData(MessageBuffer& out, const LevelData& level);
    void writeMineData(MessageBuffer& out, const MineData& mine);
    void writeCropData(MessageBuffer& out, const CropData& crop);
    void writeAnimalData(MessageBuffer& out, const AnimalData& animal);
    void writeFurnitureData(MessageBuffer& out, const FurnitureData& f);
    void writeBackgroundData(MessageBuffer& out, const BackgroundData& bg);
    void writeFarmData(MessageBuffer& out, const FarmData& farm);
    void writeZoneData(MessageBuffer& out, Zone currentZone, const ZoneData& zoneData);
    void writeGameInfo(MessageBuffer& out, const GameInfoDTO& info);
    void writePlayerData(MessageBuffer& out, const PlayerData& data);
    void writePlayerDataLocal(MessageBuffer& out, const PlayerDataLocal& localData);

public:
    explicit ServerProtocol(Socket&& socket);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;

    void shutdown();

    MessageType receiveType();
    SelectSlotDTO receiveSelectSlot();
    ItemTransferDTO receiveItemTransfer();
    SelectItemDTO receiveSelectItem();
    CloseChestDTO receiveCloseChest();
    CreateGameDTO receiveCreateGame();
    JoinGameDTO receiveJoinGame();
    ResumeGameDTO receiveResumeGame();
    MoveDTO receiveMove();
    InteractDTO receiveInteract();
    DayOkDTO receiveDayOk();
    DayCancelDTO receiveDayCancel();
    DonateDTO receiveDonate();
    ChatDTO receiveChat();
    UseItemDTO receiveUseItem();
    RequestMultiplayerCodeDTO receiveRequestMultiplayerCode();
    DisconnectDTO receiveDisconnect();
    ListGamesDTO receiveListGames();
    CheckCodeDTO receiveCheckCode();

    void sendStatus(const ServerStatus& status);
    void sendConstMap(const ConstMapData& map);
    void sendQtStatus(const qtStatus& status);
};

#endif

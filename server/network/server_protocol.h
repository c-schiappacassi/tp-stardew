#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "dtos.h"
#include "protocol_base.h"
#include "protocol_types.h"
#include "socket.h"


class ServerProtocol {
private:
    ProtocolBase base;

    Position receivePosition();
    void writePosition(MessageBuffer& out, const Position& pos);

    Color receiveColor();
    void writeColor(MessageBuffer& out, const Color& color);

    Direction receiveDirection();

    void writeStack(MessageBuffer& out, const Stack& stack);
    void writeMineralData(MessageBuffer& out, const MineralData& mineral);
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
    CreateGameDTO receiveCreateGame();
    JoinGameDTO receiveJoinGame();
    ResumeGameDTO receiveResumeGame();
    MoveDTO receiveMove();
    InteractDTO receiveInteract();
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

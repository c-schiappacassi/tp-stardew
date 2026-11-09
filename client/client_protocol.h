#pragma once

#include <optional>

#include "dtos.h"
#include "protocol_base.h"
#include "protocol_types.h"
#include "socket.h"


class ClientProtocol {
private:
    ProtocolBase base;

    Position receivePosition();
    void writePosition(MessageBuffer& out, const Position& pos);

    void writeColor(MessageBuffer& out, const Color& color);
    Color receiveColor();

    Stack receiveStack();

    MineralData receiveMineralData();
    LevelData receiveLevelData();
    MineData receiveMineData();
    ZoneData receiveZoneData(Zone currentZone);

    GameInfoDTO receiveGameInfo();

    PlayerData receivePlayerData();
    PlayerDataLocal receivePlayerDataLocal();

    CropData receiveCropData();
    AnimalData receiveAnimalData();
    DecorData receiveDecorationData();
    FarmData receiveFarmData();

public:
    explicit ClientProtocol(Socket&& socket);

    ClientProtocol(const ClientProtocol&) = delete;
    ClientProtocol& operator=(const ClientProtocol&) = delete;
    ClientProtocol(ClientProtocol&&) = default;
    ClientProtocol& operator=(ClientProtocol&&) = default;

    void shutdown();

    void send(const CommandDTO& dto);
    void send(const SelectSlotDTO& dto);
    void send(const CreateGameDTO& dto);
    void send(const JoinGameDTO& dto);
    void send(const ResumeGameDTO& dto);
    void send(const MoveDTO& dto);
    void send(const InteractDTO& dto);
    void send(const DonateDTO& dto);
    void send(const ChatDTO& dto);
    void send(const UseItemDTO& dto);
    void send(const RequestMultiplayerCodeDTO& dto);
    void send(const DisconnectDTO& dto);
    void send(const ListGamesDTO& dto);
    void send(const CheckCodeDTO& dto);

    void recv(ServerStatus& status);
    void recv(qtStatus& status);
};

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
    void sendPosition(const Position& pos);

    void sendColor(const Color& color);
    Color receiveColor();

    Stack receiveStack();

    MineralData receiveMineralData();
    LevelData receiveLevelData();
    MineData receiveMineData();
    ZoneData receiveZoneData(Zone currentZone);

    PlayerData receivePlayerData();
    PlayerDataLocal receivePlayerDataLocal();

public:
    explicit ClientProtocol(Socket& socket);

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

    void recv(ServerStatus& status);
};

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
    void sendPosition(const Position& pos);

    Color receiveColor();
    void sendColor(const Color& color);

    void sendStack(const Stack& stack);

    void sendPlayerData(const PlayerData& data);
    void sendPlayerDataLocal(const PlayerDataLocal& localData);

public:
    explicit ServerProtocol(Socket& socket);

    MessageType receiveType();

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

    void sendStatus(const ServerStatus& status);
};

#endif
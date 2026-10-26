#pragma once

#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "../common/dtos.h"
#include "../common/protocol_base.h"
#include "../common/protocol_types.h"
#include "../common/socket.h"


class ClientProtocol {
private:
    ProtocolBase base;

    Position receivePosition();

public:
    explicit ClientProtocol(Socket& socket);

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

#endif

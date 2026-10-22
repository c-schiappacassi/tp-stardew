#pragma once

#include "dtos.h"
#include "protocol_base.h"
#include "protocol_types.h"
#include "socket.h"

class ClientProtocol {
private:
    ProtocolBase base;

public:
    explicit ClientProtocol(Socket& socket);

    void send(const CreateGameDTO& dto);
    void send(const JoinGameDTO& dto);
    /*
    void send();
    void send(const ResumeGameDTO& dto);
    */

    void recv(ServerStatus& status);
};
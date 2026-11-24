#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include <memory>

#include "game/game_types.h"
#include "server_protocol.h"
#include "socket.h"
#include "thread.h"

class ClientSender : public Thread {
private:
    ServerProtocol& protocol;
    std::shared_ptr<OutQueue> output;

public:
    ClientSender(ServerProtocol& protocol, std::shared_ptr<OutQueue> output);
    void run() override;
};

#endif

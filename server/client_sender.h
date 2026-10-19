#ifndef CLIENT_SENDER_H
#define CLIENT_SENDER_H

#include <memory>

#include "../common/thread.h"

#include "game_types.h"
#include "server_protocol.h"


class ClientSender : public Thread {
private:
    ServerProtocol& protocol;
    std::shared_ptr<OutQueue> outQueue;

    void sendMessage(const OutgoingMessage& message);

public:
    ClientSender(ServerProtocol& protocol, const std::shared_ptr<OutQueue>& outQueue);

    void run() override;
};

#endif

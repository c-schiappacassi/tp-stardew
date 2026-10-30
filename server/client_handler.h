#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>

#include "game_types.h"
#include "server_protocol.h"
#include "socket.h"
#include "thread.h"


class ClientHandler : public Thread {
private:
    Socket socket;
    ServerProtocol protocol;
    GameManager& manager;
    std::shared_ptr<OutQueue> outQueue;
    GameSession session;
    std::unique_ptr<ClientSender> sender;

    bool inGame() const;

    void handleCreate();
    void handleJoin();
    void handleMove();

    void answer(const GameSession& result);

    void disconnect();

public:
    ClientHandler(Socket&& socket, GameManager& manager);

    void run() override;
    void stop() override;
};

#endif

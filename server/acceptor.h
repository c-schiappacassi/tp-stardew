#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <list>
#include <memory>

#include "../common/socket.h"
#include "../common/thread.h"

#include "client_handler.h"
#include "game_manager.h"


class Acceptor : public Thread {
private:
    Socket& listenSocket;
    GameManager& manager;
    std::list<std::unique_ptr<ClientHandler>> clients;

    void reap();
    void clear();

public:
    Acceptor(Socket& listenSocket, GameManager& manager);

    void run() override;
    void stop() override;
};

#endif

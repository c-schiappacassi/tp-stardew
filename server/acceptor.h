#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <list>
#include <memory>

#include "../common/socket.h"
#include "../common/thread.h"

#include "client_handler.h"

class Game;

class Acceptor : public Thread {
private:
    Socket& listenSocket;
    Game& game;
    std::list<std::unique_ptr<ClientHandler>> clients;

    void reap();
    void clear();

public:
    Acceptor(Socket& listenSocket, Game& game);

    void run() override;
    void stop() override;
};

#endif

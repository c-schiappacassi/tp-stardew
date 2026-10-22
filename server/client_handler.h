#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>

#include "client_sender.h"
#include "game_manager.h" // agus
#include "game_types.h"   // agus
#include "server_protocol.h"
#include "socket.h"
#include "thread.h"


class ClientHandler : public Thread {
private:
    // agus fijate si los nombres del game son otros y estos no te sirven no se que implementaste!
    Socket socket;
    ServerProtocol protocol;
    GameManager& manager;
    std::shared_ptr<OutQueue> outQueue;
    GameSession session;
    std::unique_ptr<ClientSender> sender;


    bool runMenu();
    void runGame();
    bool answerEntry();
    void disconnect();

public:
    ClientHandler(Socket&& socket, GameManager& manager);

    void run() override;
    void stop() override;
};

#endif

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>
#include <functional>
#include <unordered_map>

#include "client_sender.h"
#include "game/game_types.h"
#include "server_protocol.h"
#include "socket.h"
#include "thread.h"

class Game;

class ClientHandler : public Thread {
private:
    Socket socket;
    ServerProtocol protocol;
    Game& game;
    std::shared_ptr<OutQueue> outQueue;
    GameSession session;
    std::unique_ptr<ClientSender> sender;

    using Receiver = std::function<std::shared_ptr<CommandDTO>()>;

    std::unordered_map<MessageType, Receiver> gameReceivers;
    std::unordered_map<MessageType, std::function<void()>> menuActions;

    template <typename DTO>
    Receiver receiver(DTO (ServerProtocol::*recv)());


    bool runMenu();
    void runGame();
    bool answerEntry();
    void disconnect();

public:
    ClientHandler(Socket&& socket, Game& game);

    void run() override;
    void stop() override;
};

#endif

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

#include "game/game_manager.h"
#include "client_sender.h"
#include "game/game_types.h"
#include "protocol_types.h"
#include "server_protocol.h"
#include "socket.h"
#include "thread.h"


class ClientHandler : public Thread {
private:
    using Receiver = std::function<std::shared_ptr<CommandDTO>()>;
    using Action = std::function<void()>;

    ServerProtocol protocol;
    GameManager& manager;
    std::shared_ptr<OutQueue> outQueue;
    GameSession session;
    std::unique_ptr<ClientSender> sender;

    std::map<MessageType, Receiver> gameReceivers;
    std::map<MessageType, Action> menuActions;
    std::map<MessageType, Action> qtActions;

    template <typename DTO>
    Receiver receiver(DTO (ServerProtocol::*recv)());


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

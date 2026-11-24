#include "client_handler.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "game/game_manager.h"
#include "liberror.h"
#include "protocol_errors.h"
#include "queue.h"


template <typename DTO>
ClientHandler::Receiver ClientHandler::receiver(DTO (ServerProtocol::*recv)()) {
    return [this, recv]() { return std::make_shared<DTO>((protocol.*recv)()); };
}

ClientHandler::ClientHandler(Socket&& socket, GameManager& manager):
        protocol(std::move(socket)),
        manager(manager),
        outQueue(std::make_shared<OutQueue>()),
        session(),
        sender() {
    gameReceivers = {
        {MessageType::Move, receiver(&ServerProtocol::receiveMove)},
        {MessageType::SelectSlot, receiver(&ServerProtocol::receiveSelectSlot)},
        {MessageType::Interact, receiver(&ServerProtocol::receiveInteract)},
        {MessageType::ItemTransfer, receiver(&ServerProtocol::receiveItemTransfer)},
        {MessageType::SelectItem, receiver(&ServerProtocol::receiveSelectItem)},
        {MessageType::CloseChest, receiver(&ServerProtocol::receiveCloseChest)},
        {MessageType::DayOk, receiver(&ServerProtocol::receiveDayOk)},
        {MessageType::DayCancel, receiver(&ServerProtocol::receiveDayCancel)},
        {MessageType::UseItem, receiver(&ServerProtocol::receiveUseItem)},
        {MessageType::Donate, receiver(&ServerProtocol::receiveDonate)},
        {MessageType::Chat, receiver(&ServerProtocol::receiveChat)},
    };
    menuActions = {
        {MessageType::CreateGame, [this] { session = this->manager.createGame(protocol.receiveCreateGame(), outQueue); }},
        {MessageType::JoinGame,   [this] { session = this->manager.joinGame(protocol.receiveJoinGame(), outQueue); }},
        {MessageType::ResumeGame, [this] { session = this->manager.resumeGame(protocol.receiveResumeGame(), outQueue); }},
    };
    qtActions = {
        {MessageType::ListGames, [this] {
            const ListGamesDTO dto = protocol.receiveListGames();
            qtStatus reply;
            reply.status = StatusCode::Ok;
            reply.games = this->manager.listGames(dto.userName);
            protocol.sendQtStatus(reply);
        }},
        {MessageType::CheckCode, [this] {
            const CheckCodeDTO dto = protocol.receiveCheckCode();
            qtStatus reply;
            reply.status = StatusCode::Ok;
            if (!this->manager.codeExists(dto.code)) {
                reply.status = StatusCode::Error;
                reply.errorMsg = "invalid code";
            }
            protocol.sendQtStatus(reply);
        }},
    };
}

void ClientHandler::run() {
    try {
        if (runMenu()) {
            runGame();
        }
    } catch (const ConnectionClosed&) {
    } catch (const ClosedQueue&) {
    } catch (const LibError& e) {
        if (should_keep_running()) {
            std::cerr << "Socket error: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
    }
    disconnect();
}

bool ClientHandler::runMenu() {
    while (should_keep_running()) {
        const auto type = protocol.receiveType();
        if (type == MessageType::ExitGame) {
            protocol.receiveDisconnect();
            return false;
        }

        const auto query = qtActions.find(type);
        if (query != qtActions.end()) {
            query->second();
            continue;
        }

        const auto it = menuActions.find(type);
        if (it == menuActions.end()) throw ProtocolError("unexpected message");
        it->second();
        if (answerEntry()) return true;
    }
    return false;
}

void ClientHandler::runGame() {
    sender = std::make_unique<ClientSender>(protocol, outQueue);
    sender->start();

    while (should_keep_running()) {
        const auto type = protocol.receiveType();
        if (type == MessageType::ExitGame) {
            protocol.receiveDisconnect();
            return;
        }
        const auto it = gameReceivers.find(type);
        if (it == gameReceivers.end()) throw ProtocolError("unexpected message");
        if (manager.enqueueCommand(session.gameId, session.playerId, it->second()) != CommandSubmission::Enqueued)
            return;
    }
}

bool ClientHandler::answerEntry() {
    const bool joined = session.status.status == StatusCode::Ok;
    protocol.sendStatus(session.status);
    if (joined) {
        protocol.sendConstMap(*session.map);
    }
    if (!joined) session = GameSession();
    return joined;
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        protocol.shutdown();
    } catch (const std::exception&) {
    }
}

void ClientHandler::disconnect() {
    stop();
    if (session.status.status == StatusCode::Ok && session.playerId != 0)
        manager.enqueueCommand(session.gameId, session.playerId, std::make_shared<DisconnectDTO>(session.userName));
    try {
        outQueue->close();
    } catch (const std::exception&) {
    }
    if (sender) {
        sender->join();
    }
}

#include "client_handler.h"

#include <iostream>
#include <utility>

#include <sys/socket.h>

#include "game_manager.h"
#include "protocol_errors.h"

ClientHandler::ClientHandler(Socket&& socket, GameManager& manager):
        socket(std::move(socket)),
        protocol(this->socket),
        manager(manager),
        outQueue(std::make_shared<OutQueue>()),
        session() {}

void ClientHandler::run() {
    try {
        while (should_keep_running()) {
            switch (protocol.receiveType()) {
                case MessageType::CreateGame:
                    handleCreate();
                    break;
                case MessageType::JoinGame:
                    handleJoin();
                    break;
                case MessageType::Move:
                    handleMove();
                    break;
                case MessageType::ListGames:
                    handleListGames();
                    break;
                case MessageType::CheckCode:
                    handleCheckCode();
                    break;
                case MessageType::ExitGame:
                    stop();
                    break;
                default:
                    throw ProtocolError("unexpected message");
            }
        }
    } catch (const ConnectionClosed&) {
    } catch (const std::exception& e) {
        if (should_keep_running()) {
            std::cerr << "ClientHandler error: " << e.what() << "\n";
        }
    }
    disconnect();
}

bool ClientHandler::inGame() const { return session.input != nullptr; }

void ClientHandler::handleCreate() {
    CreateGameDTO dto = protocol.receiveCreateGame();
    if (inGame()) {
        throw ProtocolError("ya estas en una partida");
    }
    answer(manager.createGame(dto, outQueue));
}

void ClientHandler::handleJoin() {
    JoinGameDTO dto = protocol.receiveJoinGame();
    if (inGame()) {
        throw ProtocolError("ya estas en una partida");
    }
    answer(manager.joinGame(dto, outQueue));
}

void ClientHandler::handleMove() {
    MoveDTO move = protocol.receiveMove();
    if (!inGame()) {
        throw ProtocolError("no estas en una partida");
    }
    session.input->push(PlayerCommand{session.playerId, std::make_shared<MoveDTO>(move)});
}

void ClientHandler::handleListGames() {
    ListGamesDTO dto = protocol.receiveListGames();
    if (inGame()) {
        throw ProtocolError("ya estas en una partida");
    }
    qtStatus status;
    status.status = StatusCode::Ok;
    status.games = manager.listGames(dto.userName);
    protocol.sendQtStatus(status);
}

void ClientHandler::handleCheckCode() {
    CheckCodeDTO dto = protocol.receiveCheckCode();
    if (inGame()) {
        throw ProtocolError("ya estas en una partida");
    }
    qtStatus status;
    status.status = StatusCode::Ok;
    if (!manager.codeExists(dto.code)) {
        status.status = StatusCode::Error;
        status.errorMsg = "codigo invalido";
    }
    protocol.sendQtStatus(status);
}

void ClientHandler::answer(const GameSession& result) {
    protocol.sendStatus(result.status);
    if (result.input) {
        session = result;
        sender = std::make_unique<ClientSender>(protocol, outQueue);
        sender->start();
    }
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {}
}

void ClientHandler::disconnect() {
    stop();
    if (inGame()) {
        try {
            session.input->push(PlayerCommand{session.playerId, std::make_shared<DisconnectDTO>(session.userName)});
        } catch (const ClosedQueue&) {}
    }
    try {
        outQueue->close(); // esto deberia destranar el sender
    } catch (const std::exception&) {}
    if (sender) {
        sender->join();
    }
}

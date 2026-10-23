#include "client_handler.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "liberror.h"
#include "protocol_errors.h"
#include "queue.h"


struct ClosedQueue;
ClientHandler::ClientHandler(Socket&& socket, GameManager& manager)
    : socket(std::move(socket)),
      protocol(this->socket),
      manager(manager),
      outQueue(std::make_shared<OutQueue>()),
      session(),
      sender() {}


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


// despues quiero desaparecer este switch pero todavia no termino de entender como hacerlo!
bool ClientHandler::runMenu() {
    while (should_keep_running()) {
        switch (protocol.receiveType()) {

            case MessageType::CreateGame: {
                const CreateGameDTO dto = protocol.receiveCreateGame();
                session = manager.createGame(dto, outQueue); // agus
                if (answerEntry()) {
                    return true;
                }
                break;
            }
            case MessageType::JoinGame: {
                const JoinGameDTO dto = protocol.receiveJoinGame();
                session = manager.joinGame(dto, outQueue); // agus
                if (answerEntry()) {
                    return true;
                }
                break;
            }
            /* internamente seria un list y despues un resume
            case MessageType::ResumeGame: {
                const ResumeGameDTO dto = protocol.receiveResumeGame();
                session = manager.resumeGame(dto, outQueue); // agus
                if (answerEntry()) {
                    return true;
                }
                break;
            }
            */
            case MessageType::ExitGame: {
                return false;
            }
            default:
                throw ProtocolError("unexpected message");
        }
    }
    return false;
}

// este switch tambien va a volar ni bien termine de entender como usar un map
void ClientHandler::runGame() {
    sender = std::make_unique<ClientSender>(protocol, outQueue);
    sender->start();

    while (should_keep_running()) {
        switch (protocol.receiveType()) {
            // TODO: el MoveDTO ya existe. Cuando este receiveMove(), habilita este
            // encola el comando con session.playerId, como esta escrito abajo
            // por ahora el id del jugador la obtengo de su conexion, hablalo con ciro.
            /* 
            case MessageType::Move: {
                session.input->push(PlayerCommand{
                        session.playerId, protocol.receiveMove()});
                break;
            }
            */
            case MessageType::ExitGame: {
                return;
            }
            default:
                throw ProtocolError("unexpected message");
        }
    }
}


// TODO: juntar la respuesta de ingreso dentro de ServerStatus.Incorporar los datos actuales de GameJoined (gameId, playerId y worldId)
// al DTO y al protocolo, manteniendolos errores de ingreso.
// no mandar GameJoined como mensaje separado.

bool ClientHandler::answerEntry() {
    const bool joined =session.status.status == StatusCode::Ok && session.input;
    protocol.sendStatus(session.status);

    if (joined) {
        protocol.sendGameJoined(GameJoinedDTO{session.gameId, session.playerId, session.worldId});
    } else {
        session = GameSession();
    }
    return joined;
}

void ClientHandler::stop() {
    Thread::stop();
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (const std::exception&) {
    }
}

void ClientHandler::disconnect() {
    if (session.input) {
        try {
            session.input->push(PlayerCommand{session.playerId, DisconnectCommand{}});
        } catch (const ClosedQueue&) {
        }
    }
    try {
        outQueue->close();
    } catch (const std::exception&) {
    }
    if (sender) {
        sender->join();
    }
}

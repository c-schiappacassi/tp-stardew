#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "../common/dtos.h"
#include "../common/protocol_base.h"
#include "../common/protocol_types.h"
#include "../common/socket.h"

// TODO: agregar receiveMove() que devuelva MoveDTO con el destino (x, y).
// receiveType() ya usa el tipo de mensaje.
// Cambiar sendStatus() para recibir const ServerStatus& e incluir positionPlayer.

class ServerProtocol {
private:
    ProtocolBase base;

public:
    explicit ServerProtocol(Socket& socket);

    MessageType receiveType();
    CreateGameDTO receiveCreateGame();
    JoinGameDTO receiveJoinGame();
//    ResumeGameDTO receiveResumeGame();

    void sendStatus(const StatusDTO& dto);
//    void sendGameList(const GameListDTO& dto);
    void sendGameJoined(const GameJoinedDTO& dto);

};

#endif


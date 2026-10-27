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

    Position receivePosition();
    void sendPosition(const Position& pos);

    Color receiveColor();
    void sendColor(const Color& color);

    void sendPlayerData(const PlayerData& data);

public:
    explicit ServerProtocol(Socket& socket);

    MessageType receiveType();

    void receive(CreateGameDTO& dto);
    void receive(JoinGameDTO& dto);
    void receive(ResumeGameDTO& dto);
    void receive(MoveDTO& dto);
    void receive(InteractDTO& dto);
    void receive(DonateDTO& dto);
    void receive(ChatDTO& dto);
    void receive(UseItemDTO& dto);
    void receive(RequestMultiplayerCodeDTO& dto);
    void receive(DisconnectDTO& dto);

    void send(const ServerStatus& status);

    // mismo que antes, mantengo para no romper lo suyo
    void send(const StatusDTO& dto);
    void send(const GameJoinedDTO& dto);
    void send(const GameListDTO& dto);
};

#endif

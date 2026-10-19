#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "dtos.h"
#include "protocol_base.h"
#include "protocol_types.h"
#include "socket.h"

class ClientProtocol {
private:
    ProtocolBase base;

public:
    explicit ClientProtocol(Socket& socket);

    void sendCreateGame(const CreateGameDTO& dto);
    void sendJoinGame(const JoinGameDTO& dto);
    /*
    void sendListGames();
    void sendResumeGame(const ResumeGameDTO& dto);
    */

    MessageType receiveType();
    StatusDTO receiveStatus();
//    GameListDTO receiveGameList();
    GameJoinedDTO receiveGameJoined();
};

#endif

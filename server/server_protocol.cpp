#include "server_protocol.h"

#include <cstdint>

ServerProtocol::ServerProtocol(Socket& socket) : base(socket) {}

MessageType ServerProtocol::receiveType() {
    return static_cast<MessageType>(base.receiveUint8());
}

CreateGameDTO ServerProtocol::receiveCreateGame() {
    CreateGameDTO dto;
    dto.gameName = base.receiveString();
    dto.playerName = base.receiveString();
    dto.skin = base.receiveUint8();
    dto.worldId = base.receiveUint8();
    return dto;
}

JoinGameDTO ServerProtocol::receiveJoinGame() {
    JoinGameDTO dto;
    dto.code = base.receiveString();
    dto.playerName = base.receiveString();
    dto.skin = base.receiveUint8();
    return dto;
}

/*
ResumeGameDTO ServerProtocol::receiveResumeGame() {
    ResumeGameDTO dto;
    dto.gameId = base.receiveUint16();
    return dto;
}
*/

void ServerProtocol::sendStatus(const StatusDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::Status));
    base.sendUint8(static_cast<uint8_t>(dto.status));
    base.sendString(dto.errorMsg);
    base.sendString(dto.successMsg);
}

/*
void ServerProtocol::sendGameList(const GameListDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::GameList));
    base.sendUint16(static_cast<uint16_t>(dto.games.size()));
    for (const GameInfoDTO& info : dto.games) {
        base.sendUint16(info.id);
        base.sendString(info.gameName);
        base.sendString(info.playerName);
        base.sendUint8(info.skin);
        base.sendUint8(info.isMultiplayer ? 1 : 0);
    }
}
*/

void ServerProtocol::sendGameJoined(const GameJoinedDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::GameJoined));
    base.sendUint16(dto.playerId);
    base.sendUint8(dto.worldId);
}

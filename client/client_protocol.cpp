#include "client_protocol.h"

#include <cstdint>

ClientProtocol::ClientProtocol(Socket& socket) : base(socket) {}

void ClientProtocol::sendCreateGame(const CreateGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CreateGame));
    base.sendString(dto.gameName);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
    base.sendUint8(dto.worldId);
}

void ClientProtocol::sendJoinGame(const JoinGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::JoinGame));
    base.sendString(dto.code);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
}

/*
void ClientProtocol::sendListGames() {
    base.sendUint8(static_cast<uint8_t>(MessageType::ListGames));
}

void ClientProtocol::sendResumeGame(const ResumeGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    base.sendUint16(dto.gameId);
}
*/

MessageType ClientProtocol::receiveType() {
    return static_cast<MessageType>(base.receiveUint8());
}

StatusDTO ClientProtocol::receiveStatus() {
    StatusDTO dto;
    dto.status = static_cast<StatusCode>(base.receiveUint8());
    dto.errorMsg = base.receiveString();
    dto.successMsg = base.receiveString();
    return dto;
}

/*
GameListDTO ClientProtocol::receiveGameList() {
    GameListDTO dto;
    const uint16_t count = base.receiveUint16();
    dto.games.reserve(count);
    for (uint16_t i = 0; i < count; i++) {
        GameInfoDTO info;
        info.id = base.receiveUint16();
        info.gameName = base.receiveString();
        info.playerName = base.receiveString();
        info.skin = base.receiveUint8();
        info.isMultiplayer = base.receiveUint8() != 0;
        dto.games.push_back(info);
    }
    return dto;
}
*/

GameJoinedDTO ClientProtocol::receiveGameJoined() {
    GameJoinedDTO dto;
    dto.gameId = base.receiveString();
    dto.playerId = base.receiveUint16();
    dto.worldId = base.receiveUint8();
    return dto;
}

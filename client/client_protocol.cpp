#include "client_protocol.h"

#include <cstdint>

ClientProtocol::ClientProtocol(Socket& socket) : base(socket) {}

void ClientProtocol::send(const CreateGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::CreateGame));
    base.sendString(dto.gameName);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
    base.sendUint8(dto.worldId);
}

void ClientProtocol::send(const JoinGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::JoinGame));
    base.sendString(dto.code);
    base.sendString(dto.playerName);
    base.sendUint8(dto.skin);
}

/*
void ClientProtocol::send() {
    base.sendUint8(static_cast<uint8_t>(MessageType::ListGames));
}

void ClientProtocol::send(const ResumeGameDTO& dto) {
    base.sendUint8(static_cast<uint8_t>(MessageType::ResumeGame));
    base.sendUint16(dto.gameId);
}
*/

void ClientProtocol::recv(ServerStatus& status) {
    printf("%li", (sizeof(status)));
    // TODO
}
